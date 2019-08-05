/* Plugin for AMD GCN execution.

   Copyright (C) 2013-2019 Free Software Foundation, Inc.

   Contributed by Mentor Embedded

   This file is part of the GNU Offloading and Multi Processing Library
   (libgomp).

   Libgomp is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   Libgomp is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
   FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
   more details.

   Under Section 7 of GPL version 3, you are granted additional
   permissions described in the GCC Runtime Library Exception, version
   3.1, as published by the Free Software Foundation.

   You should have received a copy of the GNU General Public License and
   a copy of the GCC Runtime Library Exception along with this program;
   see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
   <http://www.gnu.org/licenses/>.  */

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <inttypes.h>
#include <stdbool.h>
#include <limits.h>
#include <hsa.h>
#include <dlfcn.h>
#include <signal.h>
#include "libgomp-plugin.h"
#include "gomp-constants.h"
#include <elf.h>
#include "oacc-plugin.h"
#include "oacc-int.h"
#include <assert.h>

#define obstack_chunk_alloc GOMP_PLUGIN_malloc
#define obstack_chunk_free free
#include "obstack.h"

/* These probably won't be in elf.h for a while.  */
#define R_AMDGPU_NONE		0
#define R_AMDGPU_ABS32_LO	1	/* (S + A) & 0xFFFFFFFF  */
#define R_AMDGPU_ABS32_HI	2	/* (S + A) >> 32  */
#define R_AMDGPU_ABS64		3	/* S + A  */
#define R_AMDGPU_REL32		4	/* S + A - P  */
#define R_AMDGPU_REL64		5	/* S + A - P  */
#define R_AMDGPU_ABS32		6	/* S + A  */
#define R_AMDGPU_GOTPCREL	7	/* G + GOT + A - P  */
#define R_AMDGPU_GOTPCREL32_LO	8	/* (G + GOT + A - P) & 0xFFFFFFFF  */
#define R_AMDGPU_GOTPCREL32_HI	9	/* (G + GOT + A - P) >> 32  */
#define R_AMDGPU_REL32_LO	10	/* (S + A - P) & 0xFFFFFFFF  */
#define R_AMDGPU_REL32_HI	11	/* (S + A - P) >> 32  */
#define reserved		12
#define R_AMDGPU_RELATIVE64	13	/* B + A  */

/* Secure getenv() which returns NULL if running as SUID/SGID.  */
#ifndef HAVE_SECURE_GETENV
#ifdef HAVE___SECURE_GETENV
#define secure_getenv __secure_getenv
#elif defined (HAVE_UNISTD_H) && defined(HAVE_GETUID) && defined(HAVE_GETEUID) \
  && defined(HAVE_GETGID) && defined(HAVE_GETEGID)

#include <unistd.h>

/* Implementation of secure_getenv() for targets where it is not provided but
   we have at least means to test real and effective IDs. */

static char *
secure_getenv (const char *name)
{
  if ((getuid () == geteuid ()) && (getgid () == getegid ()))
    return getenv (name);
  else
    return NULL;
}

#else
#define secure_getenv getenv
#endif
#endif

struct gcn_thread
{
  int async;
};

static inline struct gcn_thread *
gcn_thread (void)
{
  return (struct gcn_thread *) GOMP_PLUGIN_acc_thread ();
}

/* As an HSA runtime is dlopened, following structure defines function
   pointers utilized by the HSA plug-in.  */

struct hsa_runtime_fn_info
{
  /* HSA runtime.  */
  hsa_status_t (*hsa_status_string_fn) (hsa_status_t status,
					const char **status_string);
  hsa_status_t (*hsa_system_get_info_fn) (hsa_system_info_t attribute,
					  void *value);
  hsa_status_t (*hsa_agent_get_info_fn) (hsa_agent_t agent,
					 hsa_agent_info_t attribute,
					 void *value);
  hsa_status_t (*hsa_isa_get_info_fn)(hsa_isa_t isa,
				      hsa_isa_info_t attribute,
				      uint32_t index,
				      void *value);
  hsa_status_t (*hsa_init_fn) (void);
  hsa_status_t (*hsa_iterate_agents_fn)
    (hsa_status_t (*callback)(hsa_agent_t agent, void *data), void *data);
  hsa_status_t (*hsa_region_get_info_fn) (hsa_region_t region,
					  hsa_region_info_t attribute,
					  void *value);
  hsa_status_t (*hsa_queue_create_fn)
    (hsa_agent_t agent, uint32_t size, hsa_queue_type_t type,
     void (*callback)(hsa_status_t status, hsa_queue_t *source, void *data),
     void *data, uint32_t private_segment_size,
     uint32_t group_segment_size, hsa_queue_t **queue);
  hsa_status_t (*hsa_agent_iterate_regions_fn)
    (hsa_agent_t agent,
     hsa_status_t (*callback)(hsa_region_t region, void *data), void *data);
  hsa_status_t (*hsa_executable_destroy_fn) (hsa_executable_t executable);
  hsa_status_t (*hsa_executable_create_fn)
    (hsa_profile_t profile, hsa_executable_state_t executable_state,
     const char *options, hsa_executable_t *executable);
  hsa_status_t (*hsa_executable_global_variable_define_fn)
    (hsa_executable_t executable, const char *variable_name, void *address);
  hsa_status_t (*hsa_executable_load_code_object_fn)
    (hsa_executable_t executable, hsa_agent_t agent,
     hsa_code_object_t code_object, const char *options);
  hsa_status_t (*hsa_executable_freeze_fn)(hsa_executable_t executable,
					   const char *options);
  hsa_status_t (*hsa_signal_create_fn) (hsa_signal_value_t initial_value,
					uint32_t num_consumers,
					const hsa_agent_t *consumers,
					hsa_signal_t *signal);
  hsa_status_t (*hsa_memory_allocate_fn) (hsa_region_t region, size_t size,
					  void **ptr);
  hsa_status_t (*hsa_memory_copy_fn)(void *dst, const void *src, size_t size);
  hsa_status_t (*hsa_memory_free_fn) (void *ptr);
  hsa_status_t (*hsa_signal_destroy_fn) (hsa_signal_t signal);
  hsa_status_t (*hsa_executable_get_symbol_fn)
    (hsa_executable_t executable, const char *module_name,
     const char *symbol_name, hsa_agent_t agent, int32_t call_convention,
     hsa_executable_symbol_t *symbol);
  hsa_status_t (*hsa_executable_symbol_get_info_fn)
    (hsa_executable_symbol_t executable_symbol,
     hsa_executable_symbol_info_t attribute, void *value);
  hsa_status_t (*hsa_executable_iterate_symbols_fn)
    (hsa_executable_t executable,
     hsa_status_t (*callback)(hsa_executable_t executable,
			      hsa_executable_symbol_t symbol, void *data),
     void *data);
  uint64_t (*hsa_queue_add_write_index_release_fn) (const hsa_queue_t *queue,
						    uint64_t value);
  uint64_t (*hsa_queue_load_read_index_acquire_fn) (const hsa_queue_t *queue);
  void (*hsa_signal_store_relaxed_fn) (hsa_signal_t signal,
				       hsa_signal_value_t value);
  void (*hsa_signal_store_release_fn) (hsa_signal_t signal,
				       hsa_signal_value_t value);
  hsa_signal_value_t (*hsa_signal_wait_acquire_fn)
    (hsa_signal_t signal, hsa_signal_condition_t condition,
     hsa_signal_value_t compare_value, uint64_t timeout_hint,
     hsa_wait_state_t wait_state_hint);
  hsa_signal_value_t (*hsa_signal_load_acquire_fn) (hsa_signal_t signal);
  hsa_status_t (*hsa_queue_destroy_fn) (hsa_queue_t *queue);

  hsa_status_t (*hsa_code_object_deserialize_fn)
    (void *serialized_code_object, size_t serialized_code_object_size,
     const char *options, hsa_code_object_t *code_object);
};

/* HSA runtime functions that are initialized in init_hsa_context.  */

static struct hsa_runtime_fn_info hsa_fns;

/* Keep the following GOMP prefixed structures in sync with respective parts of
   the compiler.  */

/* Structure describing the run-time and grid properties of an HSA kernel
   lauch.  */

struct GOMP_kernel_launch_attributes
{
  /* Number of dimensions the workload has.  Maximum number is 3.  */
  uint32_t ndim;
  /* Size of the grid in the three respective dimensions.  */
  uint32_t gdims[3];
  /* Size of work-groups in the respective dimensions.  */
  uint32_t wdims[3];
};

/* Collection of information needed for a dispatch of a kernel from a
   kernel.  */

struct GOMP_hsa_kernel_dispatch
{
  /* Pointer to a command queue associated with a kernel dispatch agent.  */
  void *queue;
  /* Pointer to reserved memory for OMP data struct copying.  */
  void *omp_data_memory;
  /* Pointer to a memory space used for kernel arguments passing.  */
  void *kernarg_address;
  /* Kernel object.  */
  uint64_t object;
  /* Synchronization signal used for dispatch synchronization.  */
  uint64_t signal;
  /* Private segment size.  */
  uint32_t private_segment_size;
  /* Group segment size.  */
  uint32_t group_segment_size;
  /* Number of children kernel dispatches.  */
  uint64_t kernel_dispatch_count;
  /* Debug purpose argument.  */
  uint64_t debug;
  /* Levels-var ICV.  */
  uint64_t omp_level;
  /* Kernel dispatch structures created for children kernel dispatches.  */
  struct GOMP_hsa_kernel_dispatch **children_dispatches;
  /* Number of threads.  */
  uint32_t omp_num_threads;
};

/* Structure of the default kernargs segment, supporting gomp_print_*.
   This will only be used if the requested space is less than 9 bytes.  */

struct kernargs {
  /* Leave space for the real kernel arguments.
     OpenACC and OpenMP only use one pointer.  */
  int64_t dummy1;
  int64_t dummy2;

  /* A pointer to struct output, below, for console output data.  */
  int64_t out_ptr;

  /* A pointer to struct heap, below.  */
  int64_t heap_ptr;

  /* Output data.  */
  struct output {
    int return_value;
    unsigned int next_output;
    struct printf_data {
      int written;
      char msg[128];
      int type;
      union {
	int64_t ivalue;
	double dvalue;
	char text[128];
      };
    } queue[1024];
    unsigned int consumed;
  } output_data;
};

/* Heap space, allocated target-side, provided for use of newlib malloc.
   Each module should have it's own heap allocated.
   Beware that heap usage increases with OpenMP teams.  */
static size_t gcn_kernel_heap_size = 100*1024*1024;  /* 100MB.  */
struct heap {
  int64_t size;
  char data[0];
};

/* GCN specific definition of asynchronous queues.  */

#define ASYNC_QUEUE_SIZE 64
#define DRAIN_QUEUE_SYNCHRONOUS_P false
#define DEBUG_QUEUES 0
#define DEBUG_THREAD_SLEEP 0
#define DEBUG_THREAD_SIGNAL 0

struct kernel_launch
{
  struct kernel_info *kernel;
  void *vars;
  struct GOMP_kernel_launch_attributes kla;
};

struct callback
{
  void (*fn)(void *);
  void *data;
};

struct queue_entry
{
  int type;
  union {
    struct kernel_launch launch;
    struct callback callback;
  } u;
};

struct goacc_asyncqueue
{
  struct agent_info *agent;
  hsa_queue_t *hsa_queue;

  pthread_t thread_drain_queue;
  pthread_mutex_t mutex;
  pthread_cond_t queue_cond_in;
  pthread_cond_t queue_cond_out;
  struct queue_entry queue[ASYNC_QUEUE_SIZE];
  int queue_first;
  int queue_n;
  int drain_queue_stop;

  int id;
  struct goacc_asyncqueue *prev;
  struct goacc_asyncqueue *next;
};

/* Part of the libgomp plugin interface.  Return the name of the accelerator,
   which is "gcn".  */

const char *
GOMP_OFFLOAD_get_name (void)
{
  return "gcn";
}

/* Part of the libgomp plugin interface.  Return the specific capabilities the
   HSA accelerator have.  */

unsigned int
GOMP_OFFLOAD_get_caps (void)
{
  /* FIXME: Enable shared memory for APU, but not discrete GPU.  */
  return /*GOMP_OFFLOAD_CAP_SHARED_MEM |*/ GOMP_OFFLOAD_CAP_OPENMP_400
	    | GOMP_OFFLOAD_CAP_OPENACC_200;
}

/* Part of the libgomp plugin interface.  Identify as HSA accelerator.  */

int
GOMP_OFFLOAD_get_type (void)
{
  return OFFLOAD_TARGET_TYPE_GCN;
}

/* Return the libgomp version number we're compatible with.  There is
   no requirement for cross-version compatibility.  */

unsigned
GOMP_OFFLOAD_version (void)
{
  return GOMP_VERSION;
}

/* Flag to decide whether print to stderr information about what is going on.
   Set in init_debug depending on environment variables.  */

static bool debug;

/* Flag to decide if the runtime should suppress a possible fallback to host
   execution.  */

static bool suppress_host_fallback;

/* Flag to locate HSA runtime shared library that is dlopened
   by this plug-in.  */

static const char *hsa_runtime_lib;

/* Flag to decide if the runtime should support also CPU devices (can be
   a simulator).  */

static bool support_cpu_devices;

/* Runtime dimension overrides.  Zero indicates default.  */

static int override_x_dim = 0;
static int override_z_dim = 0;

/* Initialize debug and suppress_host_fallback according to the environment.  */

static void
init_environment_variables (void)
{
  if (secure_getenv ("GCN_DEBUG"))
    debug = true;
  else
    debug = false;

  if (secure_getenv ("GCN_SUPPRESS_HOST_FALLBACK"))
    suppress_host_fallback = true;
  else
    suppress_host_fallback = false;

  hsa_runtime_lib = secure_getenv ("HSA_RUNTIME_LIB");
  if (hsa_runtime_lib == NULL)
    hsa_runtime_lib = HSA_RUNTIME_LIB "libhsa-runtime64.so";

  support_cpu_devices = secure_getenv ("GCN_SUPPORT_CPU_DEVICES");

  const char *x = secure_getenv ("GCN_NUM_TEAMS");
  if (!x)
    x = secure_getenv ("GCN_NUM_GANGS");
  if (x)
    override_x_dim = atoi (x);

  const char *z = secure_getenv ("GCN_NUM_THREADS");
  if (!z)
    z = secure_getenv ("GCN_NUM_WORKERS");
  if (z)
    override_z_dim = atoi (z);

  const char *heap = secure_getenv ("GCN_HEAP_SIZE");
  if (heap)
    {
      size_t tmp = atol (heap);
      if (tmp)
	gcn_kernel_heap_size = tmp;
    }
}

/* Print a message to stderr if HSA_DEBUG value is set to true.  */

#define HSA_DPRINT(...) \
  do \
  { \
    if (debug) \
      { \
	fprintf (stderr, __VA_ARGS__); \
      } \
  } \
  while (false);

/* Flush stderr if GCN_DEBUG value is set to true.  */

#define HSA_FLUSH()				\
  do {						\
    if (debug)					\
      fflush (stderr);				\
  } while (0)

/* Print a logging message with PREFIX to stderr if HSA_DEBUG value
   is set to true.  */

#define HSA_LOG(prefix, ...)			\
  do						\
    {						\
      HSA_DPRINT (prefix);			\
      HSA_DPRINT (__VA_ARGS__);			\
      HSA_FLUSH ();				\
    } while (false)

/* Print a debugging message to stderr.  */

#define HSA_DEBUG(...) HSA_LOG ("GCN debug: ", __VA_ARGS__)

/* Print a warning message to stderr.  */

#define HSA_WARNING(...) HSA_LOG ("GCN warning: ", __VA_ARGS__)

/* Print HSA warning STR with an HSA STATUS code.  */

static void
hsa_warn (const char *str, hsa_status_t status)
{
  if (!debug)
    return;

  const char *hsa_error_msg;
  hsa_fns.hsa_status_string_fn (status, &hsa_error_msg);

  fprintf (stderr, "GCN warning: %s\nRuntime message: %s\n", str,
	   hsa_error_msg);
}

/* Report a fatal error STR together with the HSA error corresponding to STATUS
   and terminate execution of the current process.  */

static void
hsa_fatal (const char *str, hsa_status_t status)
{
  const char *hsa_error_msg;
  hsa_fns.hsa_status_string_fn (status, &hsa_error_msg);
  GOMP_PLUGIN_fatal ("GCN fatal error: %s\nRuntime message: %s\n", str,
		     hsa_error_msg);
}

/* Like hsa_fatal, except only report error message, and return FALSE
   for propagating error processing to outside of plugin.  */

static bool
hsa_error (const char *str, hsa_status_t status)
{
  const char *hsa_error_msg;
  hsa_fns.hsa_status_string_fn (status, &hsa_error_msg);
  GOMP_PLUGIN_error ("GCN fatal error: %s\nRuntime message: %s\n", str,
		     hsa_error_msg);
  return false;
}

struct hsa_kernel_description
{
  const char *name;
  unsigned omp_data_size;
  bool gridified_kernel_p;
  unsigned kernel_dependencies_count;
  const char **kernel_dependencies;
  int oacc_dims[3];  /* Only present for GCN kernels.  */
};

struct global_var_info
{
  const char *name;
  void *address;
};

/* Data passed by the static initializer of a compilation unit containing GCN
   object code to GOMP_offload_register.  */

struct gcn_image_desc
{
  union {
    struct gcn_image {
      char magic[4];  /* Will be "GCN" for GCN code objects.  */
      size_t size;
      void *image;
    } *gcn_image;
  };
  const unsigned kernel_count;
  struct hsa_kernel_description *kernel_infos;
  const unsigned global_variable_count;
  struct global_var_info *global_variables;
};

struct agent_info;

/* Information required to identify, finalize and run any given kernel.  */

struct kernel_info
{
  /* Name of the kernel, required to locate it within the GCN object-code
     module.  */
  const char *name;
  /* Size of memory space for OMP data.  */
  unsigned omp_data_size;
  /* The specific agent the kernel has been or will be finalized for and run
     on.  */
  struct agent_info *agent;
  /* The specific module where the kernel takes place.  */
  struct module_info *module;
  /* Mutex enforcing that at most once thread ever initializes a kernel for
     use.  A thread should have locked agent->module_rwlock for reading before
     acquiring it.  */
  pthread_mutex_t init_mutex;
  /* Flag indicating whether the kernel has been initialized and all fields
     below it contain valid data.  */
  bool initialized;
  /* Flag indicating that the kernel has a problem that blocks an execution.  */
  bool initialization_failed;
  /* The object to be put into the dispatch queue.  */
  uint64_t object;
  /* Required size of kernel arguments.  */
  uint32_t kernarg_segment_size;
  /* Required size of group segment.  */
  uint32_t group_segment_size;
  /* Required size of private segment.  */
  uint32_t private_segment_size;
  /* List of all kernel dependencies.  */
  const char **dependencies;
  /* Number of dependencies.  */
  unsigned dependencies_count;
  /* Maximum OMP data size necessary for kernel from kernel dispatches.  */
  unsigned max_omp_data_size;
  /* True if the kernel is gridified.  */
  bool gridified_kernel_p;
};

/* Information about a particular GCN module, its image and kernels.  */

struct module_info
{
  /* The description with which the program has registered the image.  */
  struct gcn_image_desc *image_desc;
  /* GCN heap allocation.  */
  struct heap *heap;
  /* Physical boundaries of the loaded module.  */
  Elf64_Addr phys_address_start;
  Elf64_Addr phys_address_end;

  bool constructors_run_p;
  struct kernel_info *init_array_func, *fini_array_func;

  /* Number of kernels in this module.  */
  int kernel_count;
  /* An array of kernel_info structures describing each kernel in this
     module.  */
  struct kernel_info kernels[];
};

/* Description of an HSA GPU agent and the program associated with it.  */

struct agent_info
{
  /* The HSA ID of the agent.  Assigned when hsa_context is initialized.  */
  hsa_agent_t id;
  /* The user-visible device number.  */
  int device_id;
  /* Whether the agent has been initialized.  The fields below are usable only
     if it has been.  */
  bool initialized;
  /* Precomuted check for problem architectures.  */
  bool gfx900_p;

  /* Command queues of the agent.  */
  hsa_queue_t *sync_queue;
  struct goacc_asyncqueue *async_queues, *omp_async_queue;
  pthread_mutex_t async_queues_mutex;

  /* The HSA memory region from which to allocate kernel arguments.  */
  hsa_region_t kernarg_region;

  /* Read-write lock that protects kernels which are running or about to be run
     from interference with loading and unloading of images.  Needs to be
     locked for reading while a kernel is being run, and for writing if the
     list of modules is manipulated (and thus the HSA program invalidated).  */
  pthread_rwlock_t module_rwlock;

  /* The module associated with this kernel.  */
  struct module_info *module;

  /* Mutex enforcing that only one thread will finalize the HSA program.  A
     thread should have locked agent->module_rwlock for reading before
     acquiring it.  */
  pthread_mutex_t prog_mutex;
  /* Flag whether the HSA program that consists of all the modules has been
     finalized.  */
  bool prog_finalized;
  /* HSA executable - the finalized program that is used to locate kernels.  */
  hsa_executable_t executable;
};

static bool create_and_finalize_hsa_program (struct agent_info *);

/* Information about the whole HSA environment and all of its agents.  */

struct hsa_context_info
{
  /* Whether the structure has been initialized.  */
  bool initialized;
  /* Number of usable GPU HSA agents in the system.  */
  int agent_count;
  /* Array of agent_info structures describing the individual HSA agents.  */
  struct agent_info *agents;
};

/* Information about the whole HSA environment and all of its agents.  */

static struct hsa_context_info hsa_context;

static bool
init_hsa_runtime_functions (void)
{
#define DLSYM_FN(function) \
  hsa_fns.function##_fn = dlsym (handle, #function); \
  if (hsa_fns.function##_fn == NULL) \
    return false;
  void *handle = dlopen (hsa_runtime_lib, RTLD_LAZY);
  if (handle == NULL)
    return false;

  DLSYM_FN (hsa_status_string)
  DLSYM_FN (hsa_system_get_info)
  DLSYM_FN (hsa_agent_get_info)
  DLSYM_FN (hsa_init)
  DLSYM_FN (hsa_iterate_agents)
  DLSYM_FN (hsa_region_get_info)
  DLSYM_FN (hsa_queue_create)
  DLSYM_FN (hsa_agent_iterate_regions)
  DLSYM_FN (hsa_executable_destroy)
  DLSYM_FN (hsa_executable_create)
  DLSYM_FN (hsa_executable_global_variable_define)
  DLSYM_FN (hsa_executable_load_code_object)
  DLSYM_FN (hsa_executable_freeze)
  DLSYM_FN (hsa_signal_create)
  DLSYM_FN (hsa_memory_allocate)
  DLSYM_FN (hsa_memory_copy)
  DLSYM_FN (hsa_memory_free)
  DLSYM_FN (hsa_signal_destroy)
  DLSYM_FN (hsa_executable_get_symbol)
  DLSYM_FN (hsa_executable_symbol_get_info)
  DLSYM_FN (hsa_executable_iterate_symbols)
  DLSYM_FN (hsa_queue_add_write_index_release)
  DLSYM_FN (hsa_queue_load_read_index_acquire)
  DLSYM_FN (hsa_signal_wait_acquire)
  DLSYM_FN (hsa_signal_store_relaxed)
  DLSYM_FN (hsa_signal_store_release)
  DLSYM_FN (hsa_signal_load_acquire)
  DLSYM_FN (hsa_queue_destroy)
  DLSYM_FN (hsa_code_object_deserialize)
  return true;
#undef DLSYM_FN
}

static void
dump_hsa_system_info (void)
{
  hsa_status_t status;

  hsa_endianness_t endianness;
  status = hsa_fns.hsa_system_get_info_fn (HSA_SYSTEM_INFO_ENDIANNESS,
					   &endianness);
  if (status == HSA_STATUS_SUCCESS)
    switch (endianness)
      {
      case HSA_ENDIANNESS_LITTLE:
	HSA_DEBUG ("HSA_SYSTEM_INFO_ENDIANNESS: LITTLE\n");
	break;
      case HSA_ENDIANNESS_BIG:
	HSA_DEBUG ("HSA_SYSTEM_INFO_ENDIANNESS: BIG\n");
	break;
      default:
	HSA_DEBUG ("HSA_SYSTEM_INFO_ENDIANNESS: UNKNOWN\n");
      }
  else
    HSA_DEBUG ("HSA_SYSTEM_INFO_ENDIANNESS: FAILED\n");

  uint8_t extensions[128];
  status = hsa_fns.hsa_system_get_info_fn (HSA_SYSTEM_INFO_EXTENSIONS,
					   &extensions);
  if (status == HSA_STATUS_SUCCESS)
    {
      if (extensions[0] & (1 << HSA_EXTENSION_IMAGES))
	HSA_DEBUG ("HSA_SYSTEM_INFO_EXTENSIONS: IMAGES\n");
    }
  else
    HSA_DEBUG ("HSA_SYSTEM_INFO_EXTENSIONS: FAILED\n");
}

static void
dump_machine_model (hsa_machine_model_t machine_model, const char *s)
{
  switch (machine_model)
    {
    case HSA_MACHINE_MODEL_SMALL:
      HSA_DEBUG ("%s: SMALL\n", s);
      break;
    case HSA_MACHINE_MODEL_LARGE:
      HSA_DEBUG ("%s: LARGE\n", s);
      break;
    default:
      HSA_DEBUG ("%s: UNKNOWN\n", s);
      break;
    }
}

static void
dump_profile (hsa_profile_t profile, const char *s)
{
  switch (profile)
    {
    case HSA_PROFILE_FULL:
      HSA_DEBUG ("%s: FULL\n", s);
      break;
    case HSA_PROFILE_BASE:
      HSA_DEBUG ("%s: BASE\n", s);
      break;
    default:
      HSA_DEBUG ("%s: UNKNOWN\n", s);
      break;
    }
}

static void dump_hsa_regions (hsa_agent_t agent);

static hsa_status_t
dump_hsa_agent_info (hsa_agent_t agent, void *data __attribute__((unused)))
{
  hsa_status_t status;

  char buf[64];
  status = hsa_fns.hsa_agent_get_info_fn (agent, HSA_AGENT_INFO_NAME,
					  &buf);
  if (status == HSA_STATUS_SUCCESS)
    HSA_DEBUG ("HSA_AGENT_INFO_NAME: %s\n", buf);
  else
    HSA_DEBUG ("HSA_AGENT_INFO_NAME: FAILED\n");

  status = hsa_fns.hsa_agent_get_info_fn (agent, HSA_AGENT_INFO_VENDOR_NAME,
					  &buf);
  if (status == HSA_STATUS_SUCCESS)
    HSA_DEBUG ("HSA_AGENT_INFO_VENDOR_NAME: %s\n", buf);
  else
    HSA_DEBUG ("HSA_AGENT_INFO_VENDOR_NAME: FAILED\n");

  hsa_machine_model_t machine_model;
  status
    = hsa_fns.hsa_agent_get_info_fn (agent, HSA_AGENT_INFO_MACHINE_MODEL,
				     &machine_model);
  if (status == HSA_STATUS_SUCCESS)
    dump_machine_model (machine_model, "HSA_AGENT_INFO_MACHINE_MODEL");
  else
    HSA_DEBUG ("HSA_AGENT_INFO_MACHINE_MODEL: FAILED\n");

  hsa_profile_t profile;
  status = hsa_fns.hsa_agent_get_info_fn (agent, HSA_AGENT_INFO_PROFILE,
					  &profile);
  if (status == HSA_STATUS_SUCCESS)
    dump_profile (profile, "HSA_AGENT_INFO_PROFILE");
  else
    HSA_DEBUG ("HSA_AGENT_INFO_PROFILE: FAILED\n");

  hsa_device_type_t device_type;
  status = hsa_fns.hsa_agent_get_info_fn (agent, HSA_AGENT_INFO_DEVICE,
					  &device_type);
  if (status == HSA_STATUS_SUCCESS)
    {
      switch (device_type)
	{
	case HSA_DEVICE_TYPE_CPU:
	  HSA_DEBUG ("HSA_AGENT_INFO_DEVICE: CPU\n");
	  break;
	case HSA_DEVICE_TYPE_GPU:
	  HSA_DEBUG ("HSA_AGENT_INFO_DEVICE: GPU\n");
	  break;
	case HSA_DEVICE_TYPE_DSP:
	  HSA_DEBUG ("HSA_AGENT_INFO_DEVICE: DSP\n");
	  break;
	default:
	  HSA_DEBUG ("HSA_AGENT_INFO_DEVICE: UNKNOWN\n");
	  break;
	}
    }
  else
    HSA_DEBUG ("HSA_AGENT_INFO_DEVICE: FAILED\n");

  uint32_t size;
  status = hsa_fns.hsa_agent_get_info_fn (agent, HSA_AGENT_INFO_WAVEFRONT_SIZE,
					  &size);
  if (status == HSA_STATUS_SUCCESS)
    HSA_DEBUG ("HSA_AGENT_INFO_WAVEFRONT_SIZE: %u\n", size);
  else
    HSA_DEBUG ("HSA_AGENT_INFO_WAVEFRONT_SIZE: FAILED\n");

  uint32_t max_dim;
  status = hsa_fns.hsa_agent_get_info_fn (agent,
					  HSA_AGENT_INFO_WORKGROUP_MAX_DIM,
					  &max_dim);
  if (status == HSA_STATUS_SUCCESS)
    HSA_DEBUG ("HSA_AGENT_INFO_WORKGROUP_MAX_DIM: %u\n", max_dim);
  else
    HSA_DEBUG ("HSA_AGENT_INFO_WORKGROUP_MAX_DIM: FAILED\n");

  uint32_t max_size;
  status = hsa_fns.hsa_agent_get_info_fn (agent,
					  HSA_AGENT_INFO_WORKGROUP_MAX_SIZE,
					  &max_size);
  if (status == HSA_STATUS_SUCCESS)
    HSA_DEBUG ("HSA_AGENT_INFO_WORKGROUP_MAX_SIZE: %u\n", max_size);
  else
    HSA_DEBUG ("HSA_AGENT_INFO_WORKGROUP_MAX_SIZE: FAILED\n");

  uint32_t grid_max_dim;
  status = hsa_fns.hsa_agent_get_info_fn (agent, HSA_AGENT_INFO_GRID_MAX_DIM,
					  &grid_max_dim);
  if (status == HSA_STATUS_SUCCESS)
    HSA_DEBUG ("HSA_AGENT_INFO_GRID_MAX_DIM: %u\n", grid_max_dim);
  else
    HSA_DEBUG ("HSA_AGENT_INFO_GRID_MAX_DIM: FAILED\n");

  uint32_t grid_max_size;
  status = hsa_fns.hsa_agent_get_info_fn (agent, HSA_AGENT_INFO_GRID_MAX_SIZE,
					  &grid_max_size);
  if (status == HSA_STATUS_SUCCESS)
    HSA_DEBUG ("HSA_AGENT_INFO_GRID_MAX_SIZE: %u\n", grid_max_size);
  else
    HSA_DEBUG ("HSA_AGENT_INFO_GRID_MAX_SIZE: FAILED\n");

  dump_hsa_regions (agent);

  return HSA_STATUS_SUCCESS;
}

/* Return true if the agent is a GPU and acceptable of concurrent submissions
   from different threads.  */

static bool
suitable_hsa_agent_p (hsa_agent_t agent)
{
  hsa_device_type_t device_type;
  hsa_status_t status
    = hsa_fns.hsa_agent_get_info_fn (agent, HSA_AGENT_INFO_DEVICE,
				     &device_type);
  if (status != HSA_STATUS_SUCCESS)
    return false;

  switch (device_type)
    {
    case HSA_DEVICE_TYPE_GPU:
      break;
    case HSA_DEVICE_TYPE_CPU:
      if (!support_cpu_devices)
	return false;
      break;
    default:
      return false;
    }

  uint32_t features = 0;
  status = hsa_fns.hsa_agent_get_info_fn (agent, HSA_AGENT_INFO_FEATURE,
					  &features);
  if (status != HSA_STATUS_SUCCESS
      || !(features & HSA_AGENT_FEATURE_KERNEL_DISPATCH))
    return false;
  hsa_queue_type_t queue_type;
  status = hsa_fns.hsa_agent_get_info_fn (agent, HSA_AGENT_INFO_QUEUE_TYPE,
					  &queue_type);
  if (status != HSA_STATUS_SUCCESS
      || (queue_type != HSA_QUEUE_TYPE_MULTI))
    return false;

  return true;
}

/* Callback of hsa_iterate_agents, if AGENT is a GPU device, increment
   agent_count in hsa_context.  */

static hsa_status_t
count_gpu_agents (hsa_agent_t agent, void *data __attribute__ ((unused)))
{
  if (suitable_hsa_agent_p (agent))
    hsa_context.agent_count++;
  return HSA_STATUS_SUCCESS;
}

/* Callback of hsa_iterate_agents, if AGENT is a GPU device, assign the agent
   id to the describing structure in the hsa context.  The index of the
   structure is pointed to by DATA, increment it afterwards.  */

static hsa_status_t
assign_agent_ids (hsa_agent_t agent, void *data)
{
  if (suitable_hsa_agent_p (agent))
    {
      int *agent_index = (int *) data;
      hsa_context.agents[*agent_index].id = agent;
      ++*agent_index;
    }
  return HSA_STATUS_SUCCESS;
}

static void
finalize_async_thread (struct goacc_asyncqueue *aq)
{
  pthread_mutex_lock (&aq->mutex);
  if (aq->drain_queue_stop == 2)
    {
      pthread_mutex_unlock (&aq->mutex);
      return;
    }

  aq->drain_queue_stop = 1;

  if (DEBUG_THREAD_SIGNAL)
    HSA_DEBUG ("Signalling async thread %d:%d: cond_in\n",
	       aq->agent->device_id, aq->id);
  pthread_cond_signal (&aq->queue_cond_in);

  while (aq->drain_queue_stop != 2)
    {
      if (DEBUG_THREAD_SLEEP)
	HSA_DEBUG ("Waiting for async thread %d:%d to finish, putting thread"
		   " to sleep\n", aq->agent->device_id, aq->id);
      pthread_cond_wait (&aq->queue_cond_out, &aq->mutex);
      if (DEBUG_THREAD_SLEEP)
	HSA_DEBUG ("Waiting, woke up thread %d:%d.  Rechecking\n",
		   aq->agent->device_id, aq->id);
    }

  HSA_DEBUG ("Done waiting for async thread %d:%d\n", aq->agent->device_id,
	     aq->id);
  pthread_mutex_unlock (&aq->mutex);

  int err = pthread_join (aq->thread_drain_queue, NULL);
  if (err != 0)
    GOMP_PLUGIN_fatal ("Join async thread %d:%d: failed: %s",
		       aq->agent->device_id, aq->id, strerror (err));
  HSA_DEBUG ("Joined with async thread %d:%d\n", aq->agent->device_id, aq->id);
}

/* Initialize hsa_context if it has not already been done.
   Return TRUE on success.  */

static bool
init_hsa_context (void)
{
  hsa_status_t status;
  int agent_index = 0;

  if (hsa_context.initialized)
    return true;
  init_environment_variables ();
  if (!init_hsa_runtime_functions ())
    {
      HSA_DEBUG ("Run-time could not be dynamically opened\n");
      if (suppress_host_fallback)
	GOMP_PLUGIN_fatal ("GCN host fallback has been suppressed");
      return false;
    }
  status = hsa_fns.hsa_init_fn ();
  if (status != HSA_STATUS_SUCCESS)
    return hsa_error ("Run-time could not be initialized", status);
  HSA_DEBUG ("HSA run-time initialized for GCN\n");

  if (debug)
    dump_hsa_system_info ();

  status = hsa_fns.hsa_iterate_agents_fn (count_gpu_agents, NULL);
  if (status != HSA_STATUS_SUCCESS)
    return hsa_error ("GCN GPU devices could not be enumerated", status);
  HSA_DEBUG ("There are %i GCN GPU devices.\n", hsa_context.agent_count);

  hsa_context.agents
    = GOMP_PLUGIN_malloc_cleared (hsa_context.agent_count
				  * sizeof (struct agent_info));
  status = hsa_fns.hsa_iterate_agents_fn (assign_agent_ids, &agent_index);
  if (agent_index != hsa_context.agent_count)
    {
      GOMP_PLUGIN_error ("Failed to assign IDs to all GCN agents");
      return false;
    }

  if (debug)
    {
      status = hsa_fns.hsa_iterate_agents_fn (dump_hsa_agent_info, NULL);
      if (status != HSA_STATUS_SUCCESS)
	GOMP_PLUGIN_error ("Failed to list all HSA runtime agents");
    }

  hsa_context.initialized = true;
  return true;
}

/* Verify that hsa_context has already been initialized and return the
   agent_info structure describing device number N.  Return NULL on error.  */

static struct agent_info *
get_agent_info (int n)
{
  if (!hsa_context.initialized)
    {
      GOMP_PLUGIN_error ("Attempt to use uninitialized GCN context.");
      return NULL;
    }
  if (n >= hsa_context.agent_count)
    {
      GOMP_PLUGIN_error ("Request to operate on non-existent GCN device %i", n);
      return NULL;
    }
  if (!hsa_context.agents[n].initialized)
    {
      GOMP_PLUGIN_error ("Attempt to use an uninitialized GCN agent.");
      return NULL;
    }
  return &hsa_context.agents[n];
}

/* Callback of dispatch queues to report errors.  */

static void
queue_callback (hsa_status_t status,
		hsa_queue_t *queue __attribute__ ((unused)),
		void *data __attribute__ ((unused)))
{
  hsa_fatal ("Asynchronous queue error", status);
}

static hsa_status_t
dump_hsa_region (hsa_region_t region, void *data __attribute__((unused)))
{
  hsa_status_t status;

  hsa_region_segment_t segment;
  status = hsa_fns.hsa_region_get_info_fn (region, HSA_REGION_INFO_SEGMENT,
					   &segment);
  if (status == HSA_STATUS_SUCCESS)
    {
      if (segment == HSA_REGION_SEGMENT_GLOBAL)
	HSA_DEBUG ("HSA_REGION_INFO_SEGMENT: GLOBAL\n");
      else if (segment == HSA_REGION_SEGMENT_READONLY)
	HSA_DEBUG ("HSA_REGION_INFO_SEGMENT: READONLY\n");
      else if (segment == HSA_REGION_SEGMENT_PRIVATE)
	HSA_DEBUG ("HSA_REGION_INFO_SEGMENT: PRIVATE\n");
      else if (segment == HSA_REGION_SEGMENT_GROUP)
	HSA_DEBUG ("HSA_REGION_INFO_SEGMENT: GROUP\n");
      else
	HSA_DEBUG ("HSA_REGION_INFO_SEGMENT: UNKNOWN\n");
    }
  else
    HSA_DEBUG ("HSA_REGION_INFO_SEGMENT: FAILED\n");

  if (segment == HSA_REGION_SEGMENT_GLOBAL)
    {
      uint32_t flags;
      status
	= hsa_fns.hsa_region_get_info_fn (region, HSA_REGION_INFO_GLOBAL_FLAGS,
					  &flags);
      if (status == HSA_STATUS_SUCCESS)
	{
	  if (flags & HSA_REGION_GLOBAL_FLAG_KERNARG)
	    HSA_DEBUG ("HSA_REGION_INFO_GLOBAL_FLAGS: KERNARG\n");
	  if (flags & HSA_REGION_GLOBAL_FLAG_FINE_GRAINED)
	    HSA_DEBUG ("HSA_REGION_INFO_GLOBAL_FLAGS: FINE_GRAINED\n");
	  if (flags & HSA_REGION_GLOBAL_FLAG_COARSE_GRAINED)
	    HSA_DEBUG ("HSA_REGION_INFO_GLOBAL_FLAGS: COARSE_GRAINED\n");
	}
      else
	HSA_DEBUG ("HSA_REGION_INFO_GLOBAL_FLAGS: FAILED\n");
    }

  size_t size;
  status = hsa_fns.hsa_region_get_info_fn (region, HSA_REGION_INFO_SIZE, &size);
  if (status == HSA_STATUS_SUCCESS)
    HSA_DEBUG ("HSA_REGION_INFO_SIZE: %zu\n", size);
  else
    HSA_DEBUG ("HSA_REGION_INFO_SIZE: FAILED\n");

  status
    = hsa_fns.hsa_region_get_info_fn (region, HSA_REGION_INFO_ALLOC_MAX_SIZE,
				      &size);
  if (status == HSA_STATUS_SUCCESS)
    HSA_DEBUG ("HSA_REGION_INFO_ALLOC_MAX_SIZE: %zu\n", size);
  else
    HSA_DEBUG ("HSA_REGION_INFO_ALLOC_MAX_SIZE: FAILED\n");

  bool alloc_allowed;
  status
    = hsa_fns.hsa_region_get_info_fn (region,
				      HSA_REGION_INFO_RUNTIME_ALLOC_ALLOWED,
				      &alloc_allowed);
  if (status == HSA_STATUS_SUCCESS)
    HSA_DEBUG ("HSA_REGION_INFO_RUNTIME_ALLOC_ALLOWED: %u\n", alloc_allowed);
  else
    HSA_DEBUG ("HSA_REGION_INFO_RUNTIME_ALLOC_ALLOWED: FAILED\n");

  if (status != HSA_STATUS_SUCCESS || !alloc_allowed)
    return HSA_STATUS_SUCCESS;

  status
    = hsa_fns.hsa_region_get_info_fn (region,
				      HSA_REGION_INFO_RUNTIME_ALLOC_GRANULE,
				      &size);
  if (status == HSA_STATUS_SUCCESS)
    HSA_DEBUG ("HSA_REGION_INFO_RUNTIME_ALLOC_GRANULE: %zu\n", size);
  else
    HSA_DEBUG ("HSA_REGION_INFO_RUNTIME_ALLOC_GRANULE: FAILED\n");

  size_t align;
  status
    = hsa_fns.hsa_region_get_info_fn (region,
				      HSA_REGION_INFO_RUNTIME_ALLOC_ALIGNMENT,
				      &align);
  if (status == HSA_STATUS_SUCCESS)
    HSA_DEBUG ("HSA_REGION_INFO_RUNTIME_ALLOC_ALIGNMENT: %zu\n", align);
  else
    HSA_DEBUG ("HSA_REGION_INFO_RUNTIME_ALLOC_ALIGNMENT: FAILED\n");

  return HSA_STATUS_SUCCESS;
}

static void
dump_hsa_regions (hsa_agent_t agent)
{
  hsa_status_t status;
  status = hsa_fns.hsa_agent_iterate_regions_fn (agent,
						 dump_hsa_region,
						 NULL);
  if (status != HSA_STATUS_SUCCESS)
    hsa_error ("Dumping hsa regions failed", status);
}

/* Return malloc'd string with name of SYMBOL.  */

static char *
get_executable_symbol_name (hsa_executable_symbol_t symbol)
{
  hsa_status_t status;
  char *res;
  uint32_t len;
  const hsa_executable_symbol_info_t info_name_length
    = HSA_EXECUTABLE_SYMBOL_INFO_NAME_LENGTH;

  status = hsa_fns.hsa_executable_symbol_get_info_fn (symbol, info_name_length,
						      &len);
  if (status != HSA_STATUS_SUCCESS)
    {
      hsa_error ("Could not get length of symbol name", status);
      return NULL;
    }

  res = GOMP_PLUGIN_malloc (len + 1);

  const hsa_executable_symbol_info_t info_name
    = HSA_EXECUTABLE_SYMBOL_INFO_NAME;

  status = hsa_fns.hsa_executable_symbol_get_info_fn (symbol, info_name, res);

  if (status != HSA_STATUS_SUCCESS)
    {
      hsa_error ("Could not get symbol name", status);
      free (res);
      return NULL;
    }

  res[len] = '\0';

  return res;
}

/* Helper function for dump_executable_symbols.  */

static hsa_status_t
dump_executable_symbol (hsa_executable_t executable,
			hsa_executable_symbol_t symbol,
			void *data __attribute__((unused)))
{
  char *name = get_executable_symbol_name (symbol);

  if (name)
    {
      HSA_DEBUG ("executable symbol: %s\n", name);
      free (name);
    }

  return HSA_STATUS_SUCCESS;
}

/* Dump all global symbol in executable.  */

static void
dump_executable_symbols (hsa_executable_t executable)
{
  hsa_status_t status;
  status
    = hsa_fns.hsa_executable_iterate_symbols_fn (executable,
						 dump_executable_symbol,
						 NULL);
  if (status != HSA_STATUS_SUCCESS)
    hsa_fatal ("Could not dump HSA executable symbols", status);
}

/* Helper function for find_executable_symbol.  */

static hsa_status_t
find_executable_symbol_1 (hsa_executable_t executable,
			  hsa_executable_symbol_t symbol,
			  void *data)
{
  hsa_executable_symbol_t *res = (hsa_executable_symbol_t *)data;
  *res = symbol;
  return HSA_STATUS_INFO_BREAK;
}

/* Find a global symbol in EXECUTABLE, save to *SYMBOL and return true.  If not
   found, return false.  */

static bool
find_executable_symbol (hsa_executable_t executable,
			hsa_executable_symbol_t *symbol)
{
  hsa_status_t status;

  status
    = hsa_fns.hsa_executable_iterate_symbols_fn (executable,
						 find_executable_symbol_1,
						 symbol);
  if (status != HSA_STATUS_INFO_BREAK)
    {
      hsa_error ("Could not find executable symbol", status);
      return false;
    }

  return true;
}

/* Callback of hsa_agent_iterate_regions.  Determine if a memory REGION can be
   used for kernarg allocations and if so write it to the memory pointed to by
   DATA and break the query.  */

static hsa_status_t
get_kernarg_memory_region (hsa_region_t region, void *data)
{
  hsa_status_t status;
  hsa_region_segment_t segment;

  status = hsa_fns.hsa_region_get_info_fn (region, HSA_REGION_INFO_SEGMENT,
					   &segment);
  if (status != HSA_STATUS_SUCCESS)
    return status;
  if (segment != HSA_REGION_SEGMENT_GLOBAL)
    return HSA_STATUS_SUCCESS;

  uint32_t flags;
  status = hsa_fns.hsa_region_get_info_fn (region, HSA_REGION_INFO_GLOBAL_FLAGS,
					   &flags);
  if (status != HSA_STATUS_SUCCESS)
    return status;
  if (flags & HSA_REGION_GLOBAL_FLAG_KERNARG)
    {
      hsa_region_t *ret = (hsa_region_t *) data;
      *ret = region;
      return HSA_STATUS_INFO_BREAK;
    }
  return HSA_STATUS_SUCCESS;
}

/* Part of the libgomp plugin interface.  Return the number of HSA devices on
   the system.  */

int
GOMP_OFFLOAD_get_num_devices (void)
{
  if (!init_hsa_context ())
    return 0;
  return hsa_context.agent_count;
}

union gomp_device_property_value
GOMP_OFFLOAD_get_property (int device, int prop)
{
  struct agent_info *agent = get_agent_info (device);
  hsa_region_t region = agent->kernarg_region;

  union gomp_device_property_value propval = { .val = 0 };

  static char buf[64];
  buf[0] = '\0';
  size_t size;
  hsa_status_t status;

  switch (prop)
    {
    case GOMP_DEVICE_PROPERTY_FREE_MEMORY:
      /* Not known: fall through.  */
    case GOMP_DEVICE_PROPERTY_MEMORY:
      status = hsa_fns.hsa_region_get_info_fn (region, HSA_REGION_INFO_SIZE,
					       &size);
      propval.val = size;
      break;
      break;
    case GOMP_DEVICE_PROPERTY_NAME:
      status = hsa_fns.hsa_agent_get_info_fn (agent->id, HSA_AGENT_INFO_NAME,
					      buf);
      if (status == HSA_STATUS_SUCCESS)
	propval.ptr = buf;
      break;
    case GOMP_DEVICE_PROPERTY_VENDOR:
      status = hsa_fns.hsa_agent_get_info_fn (agent->id,
					      HSA_AGENT_INFO_VENDOR_NAME,
					      buf);
      if (status == HSA_STATUS_SUCCESS)
	propval.ptr = buf;
      break;
    case GOMP_DEVICE_PROPERTY_DRIVER:
      propval.ptr = "HSA Runtime";
      break;
    }

  return propval;
}

static void
queue_push_launch (struct goacc_asyncqueue *aq, struct kernel_info *kernel,
		   void *vars, struct GOMP_kernel_launch_attributes *kla)
{
  assert (aq->agent == kernel->agent);

  if (aq->queue_n == ASYNC_QUEUE_SIZE)
    GOMP_PLUGIN_fatal ("ran out of async queue in thread %d:%d",
		       aq->agent->device_id, aq->id);

  pthread_mutex_lock (&aq->mutex);

  int queue_last = ((aq->queue_first + aq->queue_n)
		    % ASYNC_QUEUE_SIZE);
  if (DEBUG_QUEUES)
    HSA_DEBUG ("queue_push_launch %d:%d: at %i\n", aq->agent->device_id,
	       aq->id, queue_last);

  aq->queue[queue_last].type = 0;
  aq->queue[queue_last].u.launch.kernel = kernel;
  aq->queue[queue_last].u.launch.vars = vars;
  aq->queue[queue_last].u.launch.kla = *kla;

  aq->queue_n++;

  if (DEBUG_THREAD_SIGNAL)
    HSA_DEBUG ("signalling async thread %d:%d: cond_in\n",
	       aq->agent->device_id, aq->id);
  pthread_cond_signal (&aq->queue_cond_in);

  pthread_mutex_unlock (&aq->mutex);
}

static void
queue_push_callback (struct goacc_asyncqueue *aq, void (*fn)(void *),
		     void *data)
{
  if (aq->queue_n == ASYNC_QUEUE_SIZE)
    GOMP_PLUGIN_fatal ("Async thread %d:%d: error: queue overflowed",
		       aq->agent->device_id, aq->id);

  pthread_mutex_lock (&aq->mutex);

  int queue_last = ((aq->queue_first + aq->queue_n)
		    % ASYNC_QUEUE_SIZE);
  if (DEBUG_QUEUES)
    HSA_DEBUG ("queue_push_callback %d:%d: at %i\n", aq->agent->device_id,
	       aq->id, queue_last);

  aq->queue[queue_last].type = 1;
  aq->queue[queue_last].u.callback.fn = fn;
  aq->queue[queue_last].u.callback.data = data;

  aq->queue_n++;

  if (DEBUG_THREAD_SIGNAL)
    HSA_DEBUG ("signalling async thread %d:%d: cond_in\n",
	       aq->agent->device_id, aq->id);
  pthread_cond_signal (&aq->queue_cond_in);

  pthread_mutex_unlock (&aq->mutex);
}

static void run_kernel (struct kernel_info *kernel, void *vars,
			struct GOMP_kernel_launch_attributes *kla,
			struct goacc_asyncqueue *aq, bool module_locked);

static void
execute_queue_entry (struct goacc_asyncqueue *aq, int index)
{
  struct queue_entry *entry = &aq->queue[index];
  if (entry->type == 0)
    {
      if (DEBUG_QUEUES)
	HSA_DEBUG ("Async thread %d:%d: Executing launch entry (%d)\n",
		   aq->agent->device_id, aq->id, index);
      run_kernel (entry->u.launch.kernel,
		  entry->u.launch.vars,
		  &entry->u.launch.kla, aq, false);
      if (DEBUG_QUEUES)
	HSA_DEBUG ("Async thread %d:%d: Executing launch entry (%d) done\n",
		   aq->agent->device_id, aq->id, index);
    }
  else if (entry->type == 1)
    {
      if (DEBUG_QUEUES)
	HSA_DEBUG ("Async thread %d:%d: Executing callback entry (%d)\n",
		   aq->agent->device_id, aq->id, index);
      entry->u.callback.fn (entry->u.callback.data);
      if (DEBUG_QUEUES)
	HSA_DEBUG ("Async thread %d:%d: Executing callback entry (%d) done\n",
		   aq->agent->device_id, aq->id, index);
    }
  else
    GOMP_PLUGIN_fatal ("Unknown queue element");
}

static void *
drain_queue (void *thread_arg)
{
  struct goacc_asyncqueue *aq = thread_arg;

  if (DRAIN_QUEUE_SYNCHRONOUS_P)
    {
      aq->drain_queue_stop = 2;
      return NULL;
    }

  pthread_mutex_lock (&aq->mutex);

  while (true)
    {
      if (aq->drain_queue_stop)
	break;

      if (aq->queue_n > 0)
	{
	  pthread_mutex_unlock (&aq->mutex);
	  execute_queue_entry (aq, aq->queue_first);

	  pthread_mutex_lock (&aq->mutex);
	  aq->queue_first = ((aq->queue_first + 1)
			     % ASYNC_QUEUE_SIZE);
	  aq->queue_n--;

	  if (DEBUG_THREAD_SIGNAL)
	    HSA_DEBUG ("Async thread %d:%d: broadcasting queue out update\n",
		       aq->agent->device_id, aq->id);
	  pthread_cond_broadcast (&aq->queue_cond_out);
	  pthread_mutex_unlock (&aq->mutex);

	  if (DEBUG_QUEUES)
	    HSA_DEBUG ("Async thread %d:%d: continue\n", aq->agent->device_id,
		       aq->id);
	  pthread_mutex_lock (&aq->mutex);
	}
      else
	{
	  if (DEBUG_THREAD_SLEEP)
	    HSA_DEBUG ("Async thread %d:%d: going to sleep\n",
		       aq->agent->device_id, aq->id);
	  pthread_cond_wait (&aq->queue_cond_in, &aq->mutex);
	  if (DEBUG_THREAD_SLEEP)
	    HSA_DEBUG ("Async thread %d:%d: woke up, rechecking\n",
		       aq->agent->device_id, aq->id);
	}
    }

  aq->drain_queue_stop = 2;
  if (DEBUG_THREAD_SIGNAL)
    HSA_DEBUG ("Async thread %d:%d: broadcasting last queue out update\n",
	       aq->agent->device_id, aq->id);
  pthread_cond_broadcast (&aq->queue_cond_out);
  pthread_mutex_unlock (&aq->mutex);

  HSA_DEBUG ("Async thread %d:%d: returning\n", aq->agent->device_id, aq->id);
  return NULL;
}

static void
drain_queue_synchronous (struct goacc_asyncqueue *aq)
{
  pthread_mutex_lock (&aq->mutex);

  while (aq->queue_n > 0)
    {
      execute_queue_entry (aq, aq->queue_first);

      aq->queue_first = ((aq->queue_first + 1)
			 % ASYNC_QUEUE_SIZE);
      aq->queue_n--;
    }

  pthread_mutex_unlock (&aq->mutex);
}

/* Part of the libgomp plugin interface.  Initialize agent number N so that it
   can be used for computation.  Return TRUE on success.  */

bool
GOMP_OFFLOAD_init_device (int n)
{
  if (!init_hsa_context ())
    return false;
  if (n >= hsa_context.agent_count)
    {
      GOMP_PLUGIN_error ("Request to initialize non-existent GCN device %i", n);
      return false;
    }
  struct agent_info *agent = &hsa_context.agents[n];

  if (agent->initialized)
    return true;

  agent->device_id = n;

  if (pthread_rwlock_init (&agent->module_rwlock, NULL))
    {
      GOMP_PLUGIN_error ("Failed to initialize a GCN agent rwlock");
      return false;
    }
  if (pthread_mutex_init (&agent->prog_mutex, NULL))
    {
      GOMP_PLUGIN_error ("Failed to initialize a GCN agent program mutex");
      return false;
    }
  if (pthread_mutex_init (&agent->async_queues_mutex, NULL))
    {
      GOMP_PLUGIN_error ("Failed to initialize a GCN agent queue mutex");
      return false;
    }
  agent->async_queues = NULL;
  agent->omp_async_queue = NULL;

  uint32_t queue_size;
  hsa_status_t status;
  status = hsa_fns.hsa_agent_get_info_fn (agent->id,
					  HSA_AGENT_INFO_QUEUE_MAX_SIZE,
					  &queue_size);
  if (status != HSA_STATUS_SUCCESS)
    return hsa_error ("Error requesting maximum queue size of the GCN agent",
		      status);

  char buf[64];
  status = hsa_fns.hsa_agent_get_info_fn (agent->id, HSA_AGENT_INFO_NAME,
					  &buf);
  if (status != HSA_STATUS_SUCCESS)
    return hsa_error ("Error querying the name of the agent", status);
  agent->gfx900_p = (strncmp (buf, "gfx900", 6) == 0);

  status = hsa_fns.hsa_queue_create_fn (agent->id, queue_size,
					HSA_QUEUE_TYPE_MULTI, queue_callback,
					NULL, UINT32_MAX, UINT32_MAX,
					&agent->sync_queue);
  if (status != HSA_STATUS_SUCCESS)
    return hsa_error ("Error creating command queue", status);

  agent->kernarg_region.handle = (uint64_t) -1;
  status = hsa_fns.hsa_agent_iterate_regions_fn (agent->id,
						 get_kernarg_memory_region,
						 &agent->kernarg_region);
  if (agent->kernarg_region.handle == (uint64_t) -1)
    {
      GOMP_PLUGIN_error ("Could not find suitable memory region for kernel "
			 "arguments");
      return false;
    }
  HSA_DEBUG ("Selected kernel arguments memory region:\n");
  dump_hsa_region (agent->kernarg_region, NULL);

  HSA_DEBUG ("GCN agent %d initialized\n", n);

  agent->initialized = true;
  return true;
}

/* Free the HSA program in agent and everything associated with it and set
   agent->prog_finalized and the initialized flags of all kernels to false.
   Return TRUE on success.  */

static bool
destroy_hsa_program (struct agent_info *agent)
{
  if (!agent->prog_finalized)
    return true;

  hsa_status_t status;

  HSA_DEBUG ("Destroying the current GCN program.\n");

  status = hsa_fns.hsa_executable_destroy_fn (agent->executable);
  if (status != HSA_STATUS_SUCCESS)
    return hsa_error ("Could not destroy GCN executable", status);

  if (agent->module)
    {
      int i;
      for (i = 0; i < agent->module->kernel_count; i++)
	agent->module->kernels[i].initialized = false;

      if (agent->module->heap)
	{
	  hsa_fns.hsa_memory_free_fn (agent->module->heap);
	  agent->module->heap = NULL;
	}
    }
  agent->prog_finalized = false;
  return true;
}

/* Initialize KERNEL from D and other parameters.  Return true on success. */

static bool
init_basic_kernel_info (struct kernel_info *kernel,
			struct hsa_kernel_description *d,
			struct agent_info *agent,
			struct module_info *module)
{
  kernel->agent = agent;
  kernel->module = module;
  kernel->name = d->name;
  kernel->omp_data_size = d->omp_data_size;
  kernel->gridified_kernel_p = d->gridified_kernel_p;
  kernel->dependencies_count = d->kernel_dependencies_count;
  kernel->dependencies = d->kernel_dependencies;
  if (pthread_mutex_init (&kernel->init_mutex, NULL))
    {
      GOMP_PLUGIN_error ("Failed to initialize a GCN kernel mutex");
      return false;
    }
  return true;
}

static void init_kernel (struct kernel_info *kernel);

/* Part of the libgomp plugin interface.  Load GCN object-code module
   described by struct gcn_image_desc in TARGET_DATA and return references to
   kernel descriptors in TARGET_TABLE.  */

int
GOMP_OFFLOAD_load_image (int ord, unsigned version, const void *target_data,
			 struct addr_pair **target_table)
{
  if (GOMP_VERSION_DEV (version) > GOMP_VERSION_GCN)
    {
      GOMP_PLUGIN_error ("Offload data incompatible with GCN plugin"
			 " (expected %u, received %u)",
			 GOMP_VERSION_GCN, GOMP_VERSION_DEV (version));
      return -1;
    }

  struct gcn_image_desc *image_desc = (struct gcn_image_desc *) target_data;
  struct agent_info *agent;
  struct addr_pair *pair;
  struct module_info *module;
  struct kernel_info *kernel;
  int kernel_count = image_desc->kernel_count;
  unsigned var_count = image_desc->global_variable_count;

  agent = get_agent_info (ord);
  if (!agent)
    return -1;

  if (pthread_rwlock_wrlock (&agent->module_rwlock))
    {
      GOMP_PLUGIN_error ("Unable to write-lock a GCN agent rwlock");
      return -1;
    }
  if (agent->prog_finalized
      && !destroy_hsa_program (agent))
    return -1;

  HSA_DEBUG ("Encountered %d kernels in an image\n", kernel_count);
  HSA_DEBUG ("Encountered %u global variables in an image\n", var_count);
  pair = GOMP_PLUGIN_malloc ((kernel_count + var_count - 2)
			     * sizeof (struct addr_pair));
  *target_table = pair;
  module = (struct module_info *)
    GOMP_PLUGIN_malloc_cleared (sizeof (struct module_info)
				+ kernel_count * sizeof (struct kernel_info));
  module->image_desc = image_desc;
  module->kernel_count = kernel_count;
  module->heap = NULL;
  module->constructors_run_p = false;

  kernel = &module->kernels[0];

  /* We have the magic code for a native GCN ELF kernel, not something
     else.  */
  if (strcmp (image_desc->gcn_image->magic, "GCN") != 0)
    return -1;

  /* Allocate memory for kernel dependencies.  */
  for (unsigned i = 0; i < kernel_count; i++)
    {
      struct hsa_kernel_description *d = &image_desc->kernel_infos[i];
      if (!init_basic_kernel_info (kernel, d, agent, module))
	return -1;
      if (strcmp (d->name, "_init_array") == 0)
	module->init_array_func = kernel;
      else if (strcmp (d->name, "_fini_array") == 0)
        module->fini_array_func = kernel;
      else
	{
	  pair->start = (uintptr_t) kernel;
	  pair->end = (uintptr_t) (kernel + 1);
	  pair++;
	}
      kernel++;
    }

  agent->module = module;
  if (pthread_rwlock_unlock (&agent->module_rwlock))
    {
      GOMP_PLUGIN_error ("Unable to unlock a GCN agent rwlock");
      return -1;
    }

  if (!create_and_finalize_hsa_program (agent))
    return -1;

  for (unsigned i = 0; i < var_count; i++)
    {
      struct global_var_info *v = &image_desc->global_variables[i];
      HSA_DEBUG ("Looking for variable %s\n", v->name);

      hsa_status_t status;
      hsa_executable_symbol_t var_symbol;
      status = hsa_fns.hsa_executable_get_symbol_fn (agent->executable, NULL,
						     v->name, agent->id,
						     0, &var_symbol);

      if (status != HSA_STATUS_SUCCESS)
	hsa_fatal ("Could not find symbol for variable in the code object",
		   status);

      uint64_t var_addr;
      uint32_t var_size;
      status = hsa_fns.hsa_executable_symbol_get_info_fn
	(var_symbol, HSA_EXECUTABLE_SYMBOL_INFO_VARIABLE_ADDRESS, &var_addr);
      if (status != HSA_STATUS_SUCCESS)
	hsa_fatal ("Could not extract a variable from its symbol", status);
      status = hsa_fns.hsa_executable_symbol_get_info_fn
	(var_symbol, HSA_EXECUTABLE_SYMBOL_INFO_VARIABLE_SIZE, &var_size);
      if (status != HSA_STATUS_SUCCESS)
	hsa_fatal ("Could not extract a variable size from its symbol", status);

      pair->start = var_addr;
      pair->end = var_addr + var_size;
      HSA_DEBUG ("Found variable %s at %p with size %u\n", v->name,
		 (void *)var_addr, var_size);
      pair++;
    }

  /* Ensure that constructors are run first.  */
  struct GOMP_kernel_launch_attributes kla =
    { 3,
      /* Grid size.  */
      { 1, 64, 1 },
      /* Work-group size.  */
      { 1, 64, 1 }
    };

  if (module->init_array_func)
    {
      init_kernel (module->init_array_func);
      run_kernel (module->init_array_func, NULL, &kla, NULL, false);
    }
  module->constructors_run_p = true;

  return kernel_count + var_count;
}

/* Find the load_offset for MODULE, savte to *LOAD_OFFSET, and return true.  If
   not found, return false.  */

static bool
find_load_offset (Elf64_Addr *load_offset, struct agent_info *agent,
		  struct module_info *module, Elf64_Ehdr *image,
		  Elf64_Shdr *sections)
{
  bool res = false;

  hsa_status_t status;

  hsa_executable_symbol_t symbol;
  if (!find_executable_symbol (agent->executable, &symbol))
    return false;

  status = hsa_fns.hsa_executable_symbol_get_info_fn
    (symbol, HSA_EXECUTABLE_SYMBOL_INFO_VARIABLE_ADDRESS, load_offset);
  if (status != HSA_STATUS_SUCCESS)
    {
      hsa_error ("Could not extract symbol address", status);
      return false;
    }

  char *symbol_name = get_executable_symbol_name (symbol);
  if (symbol_name == NULL)
    return false;

  /* Find the kernel function in ELF, and calculate actual load offset.  */
  for (int i = 0; i < image->e_shnum; i++)
    if (sections[i].sh_type == SHT_SYMTAB)
      {
	Elf64_Shdr *strtab = &sections[sections[i].sh_link];
	char *strings = (char *)image + strtab->sh_offset;

	for (size_t offset = 0;
	     offset < sections[i].sh_size;
	     offset += sections[i].sh_entsize)
	  {
	    Elf64_Sym *sym = (Elf64_Sym*)((char*)image
					  + sections[i].sh_offset
					  + offset);
	    if (strcmp (symbol_name, strings + sym->st_name) == 0)
	      {
		*load_offset -= sym->st_value;
		res = true;
		break;
	      }
	  }
      }

  free (symbol_name);
  return res;
}

/* Create and finalize the program consisting of all loaded modules.  */

static bool
create_and_finalize_hsa_program (struct agent_info *agent)
{
  hsa_status_t status;
  int reloc_count = 0;
  bool res = true;
  if (pthread_mutex_lock (&agent->prog_mutex))
    {
      GOMP_PLUGIN_error ("Could not lock a GCN agent program mutex");
      return false;
    }
  if (agent->prog_finalized)
    goto final;

  status
    = hsa_fns.hsa_executable_create_fn (HSA_PROFILE_FULL,
					HSA_EXECUTABLE_STATE_UNFROZEN,
					"", &agent->executable);
  if (status != HSA_STATUS_SUCCESS)
    {
      hsa_error ("Could not create GCN executable", status);
      goto fail;
    }

  struct obstack unmodified_sections_os;
  obstack_init (&unmodified_sections_os);

  /* Load any GCN modules.  */
  struct module_info *module = agent->module;
  if (module)
    {
      Elf64_Ehdr *image = (Elf64_Ehdr *)module->image_desc->gcn_image->image;

      /* Hide relocations from the HSA runtime loader.
	 Keep a copy of the unmodified section headers to use later.  */
      Elf64_Shdr *image_sections = (Elf64_Shdr *)((char *)image
						  + image->e_shoff);
      Elf64_Shdr *sections = malloc (sizeof (Elf64_Shdr) * image->e_shnum);
      memcpy (sections, image_sections, sizeof (Elf64_Shdr) * image->e_shnum);
      for (int i = image->e_shnum - 1; i >= 0; i--)
	{
	  if (image_sections[i].sh_type == SHT_RELA
	      || image_sections[i].sh_type == SHT_REL)
	    /* Change section type to something harmless.  */
	    image_sections[i].sh_type = SHT_NOTE;
	}
      obstack_ptr_grow (&unmodified_sections_os, sections);

      hsa_code_object_t co = { 0 };
      status = hsa_fns.hsa_code_object_deserialize_fn
	(module->image_desc->gcn_image->image,
	 module->image_desc->gcn_image->size,
	 NULL, &co);
      if (status != HSA_STATUS_SUCCESS)
	{
	  hsa_error ("Could not deserialize GCN code object", status);
	  goto fail;
	}

      status = hsa_fns.hsa_executable_load_code_object_fn
	(agent->executable, agent->id, co, "");
      if (status != HSA_STATUS_SUCCESS)
	{
	  hsa_error ("Could not load GCN code object", status);
	  goto fail;
	}

      if (!module->heap)
	{
	  status = hsa_fns.hsa_memory_allocate_fn (agent->kernarg_region,
						   gcn_kernel_heap_size,
						   (void**)&module->heap);
	  if (status != HSA_STATUS_SUCCESS)
	    {
	      hsa_error ("Could not allocate memory for GCN heap", status);
	      goto fail;
	    }

	  module->heap->size = gcn_kernel_heap_size;
	}

    }
  Elf64_Shdr **unmodified_sections = obstack_finish (&unmodified_sections_os);

  if (debug)
    dump_executable_symbols (agent->executable);

  status = hsa_fns.hsa_executable_freeze_fn (agent->executable, "");
  if (status != HSA_STATUS_SUCCESS)
    {
      hsa_error ("Could not freeze the GCN executable", status);
      goto fail;
    }

  int s = 0;
  if (agent->module)
    {
      struct module_info *module = agent->module;
      Elf64_Ehdr *image = (Elf64_Ehdr *)module->image_desc->gcn_image->image;
      Elf64_Shdr *sections = unmodified_sections[s++];

      Elf64_Addr load_offset;
      if (!find_load_offset (&load_offset, agent, module, image, sections))
	goto fail;

      /* Record the physical load address range.
	 We need this for data copies later.  */
      Elf64_Phdr *segments = (Elf64_Phdr *)((char*)image + image->e_phoff);
      Elf64_Addr low = ~0, high = 0;
      for (int i = 0; i < image->e_phnum; i++)
	if (segments[i].p_memsz > 0)
	  {
	    if (segments[i].p_paddr < low)
	      low = segments[i].p_paddr;
	    if (segments[i].p_paddr > high)
	      high = segments[i].p_paddr + segments[i].p_memsz - 1;
	  }
      module->phys_address_start = low + load_offset;
      module->phys_address_end = high + load_offset;

      // Find dynamic symbol table
      Elf64_Shdr *dynsym = NULL;
      for (int i = 0; i < image->e_shnum; i++)
	if (sections[i].sh_type == SHT_DYNSYM)
	  {
	    dynsym = &sections[i];
	    break;
	  }

      /* Fix up relocations.  */
      for (int i = 0; i < image->e_shnum; i++)
	{
	  if (sections[i].sh_type == SHT_RELA)
	    for (size_t offset = 0;
		 offset < sections[i].sh_size;
		 offset += sections[i].sh_entsize)
	      {
		Elf64_Rela *reloc = (Elf64_Rela*)((char*)image
						  + sections[i].sh_offset
						  + offset);
		Elf64_Sym *sym =
		  (dynsym
		   ? (Elf64_Sym*)((char*)image
				  + dynsym->sh_offset
				  + (dynsym->sh_entsize
				     * ELF64_R_SYM (reloc->r_info)))
		   : NULL);

		int64_t S = (sym ? sym->st_value : 0);
		int64_t P = reloc->r_offset + load_offset;
		int64_t A = reloc->r_addend;
		int64_t B = load_offset;
		int64_t V, size;
		switch (ELF64_R_TYPE (reloc->r_info))
		  {
		  case R_AMDGPU_ABS32_LO:
		    V = (S + A) & 0xFFFFFFFF;
		    size = 4;
		    break;
		  case R_AMDGPU_ABS32_HI:
		    V = (S + A) >> 32;
		    size = 4;
		    break;
		  case R_AMDGPU_ABS64:
		    V = S + A;
		    size = 8;
		    break;
		  case R_AMDGPU_REL32:
		    V = S + A - P;
		    size = 4;
		    break;
		  case R_AMDGPU_REL64:
		    /* FIXME
		       LLD seems to emit REL64 where the the assembler has
		       ABS64.  This is clearly wrong because it's not what the
		       compiler is expecting.  Let's assume, for now, that
		       it's a bug.  In any case, GCN kernels are always self
		       contained and therefore relative relocations will have
		       been resolved already, so this should be a safe
		       workaround.  */
		    V = S + A/* - P*/;
		    size = 8;
		    break;
		  case R_AMDGPU_ABS32:
		    V = S + A;
		    size = 4;
		    break;
		    /* TODO R_AMDGPU_GOTPCREL */
		    /* TODO R_AMDGPU_GOTPCREL32_LO */
		    /* TODO R_AMDGPU_GOTPCREL32_HI */
		  case R_AMDGPU_REL32_LO:
		    V = (S + A - P) & 0xFFFFFFFF;
		    size = 4;
		    break;
		  case R_AMDGPU_REL32_HI:
		    V = (S + A - P) >> 32;
		    size = 4;
		    break;
		  case R_AMDGPU_RELATIVE64:
		    V = B + A;
		    size = 8;
		    break;
		  default:
		    fprintf (stderr, "Error: unsupported relocation type.\n");
		    exit (1);
		  }
		status = hsa_fns.hsa_memory_copy_fn ((void*)P, &V, size);
		if (status != HSA_STATUS_SUCCESS)
		  {
		    hsa_error ("Failed to fix up relocation", status);
		    goto fail;
		  }
		reloc_count++;
	      }
	}

      free (sections);
    }
  obstack_free (&unmodified_sections_os, NULL);

  HSA_DEBUG ("Loaded GCN kernels to device %d (%d relocations)\n",
	     agent->device_id, reloc_count);

final:
  agent->prog_finalized = true;

  if (pthread_mutex_unlock (&agent->prog_mutex))
    {
      GOMP_PLUGIN_error ("Could not unlock a GCN agent program mutex");
      res = false;
    }

  return res;

fail:
  res = false;
  goto final;
}

/* Create kernel dispatch data structure for given KERNEL.  */

static struct GOMP_hsa_kernel_dispatch *
create_single_kernel_dispatch (struct kernel_info *kernel,
			       unsigned omp_data_size)
{
  struct agent_info *agent = kernel->agent;
  struct GOMP_hsa_kernel_dispatch *shadow
    = GOMP_PLUGIN_malloc_cleared (sizeof (struct GOMP_hsa_kernel_dispatch));

  shadow->omp_data_memory
    = omp_data_size > 0 ? GOMP_PLUGIN_malloc (omp_data_size) : NULL;
  unsigned dispatch_count = kernel->dependencies_count;
  if (dispatch_count != 0)
    GOMP_PLUGIN_fatal ("kernel->dependencies_count != 0");
  shadow->kernel_dispatch_count = 0;

  shadow->object = kernel->object;

  hsa_signal_t sync_signal;
  hsa_status_t status = hsa_fns.hsa_signal_create_fn (1, 0, NULL, &sync_signal);
  if (status != HSA_STATUS_SUCCESS)
    hsa_fatal ("Error creating the GCN sync signal", status);

  shadow->signal = sync_signal.handle;
  shadow->private_segment_size = kernel->private_segment_size;
  shadow->group_segment_size = kernel->group_segment_size;

  /* Ensure that there is space for the gomp_print data.
     See also gcn-run.c, in GCC.  */
  size_t kss = kernel->kernarg_segment_size;
  bool use_gomp_print = false;
  if (kss <= 8)
    {
      kss = sizeof (struct kernargs);
      use_gomp_print = true;
    }

  status
    = hsa_fns.hsa_memory_allocate_fn (agent->kernarg_region,
				      kss,
				      &shadow->kernarg_address);
  if (status != HSA_STATUS_SUCCESS)
    hsa_fatal ("Could not allocate memory for GCN kernel arguments", status);

  struct kernargs *kernargs = shadow->kernarg_address;
  if (use_gomp_print)
    {
      /* Zero-initialize the output_data (minimum needed).  */
      kernargs->out_ptr = (int64_t)&kernargs->output_data;
      kernargs->output_data.next_output = 0;
      for (unsigned i = 0;
	   i < (sizeof (kernargs->output_data.queue)
		/ sizeof (kernargs->output_data.queue[0]));
	   i++)
	kernargs->output_data.queue[i].written = 0;
      kernargs->output_data.consumed = 0;

      /* Pass in the heap location.  */
      kernargs->heap_ptr = (int64_t)kernel->module->heap;
    }

  kernargs->output_data.return_value = 0xcafe0000;

  return shadow;
}

/* Output any data written by gomp_print_*.
   Only enabled when the requested kernarg_segment_size would not
   overwrite the gomp_print data.
   We print all entries from print_index to the next entry without a "written"
   flag.  Subsequent calls should use the returned print_index value to resume
   from the same point.  */
static void
gomp_print_output (struct kernel_info *kernel, struct kernargs *kernargs,
		   bool final)
{
  if (kernel->kernarg_segment_size <= 8)
    {
      unsigned int limit = (sizeof (kernargs->output_data.queue)
			    / sizeof (kernargs->output_data.queue[0]));

      unsigned int from = __atomic_load_n (&kernargs->output_data.consumed,
					   __ATOMIC_ACQUIRE);
      unsigned int to = kernargs->output_data.next_output;

      if (from > to)
	{
	  /* Overflow.  */
	  if (final)
	    printf ("GCN print buffer overflowed.\n");
	  return;
	}

      unsigned int i;
      for (i = from; i < to; i++)
	{
	  struct printf_data *data = &kernargs->output_data.queue[i%limit];

	  if (!data->written && !final)
	    break;

	  switch (data->type)
	    {
	    case 0: printf ("%.128s%ld\n", data->msg, data->ivalue); break;
	    case 1: printf ("%.128s%f\n", data->msg, data->dvalue); break;
	    case 2: printf ("%.128s%.128s\n", data->msg, data->text); break;
	    case 3: printf ("%.128s%.128s", data->msg, data->text); break;
	    default: printf ("GCN print buffer error!\n"); break;
	    }
	  data->written = 0;
	  __atomic_store_n (&kernargs->output_data.consumed, i+1,
			    __ATOMIC_RELEASE);
	}
      fflush (stdout);
    }
}

/* Release data structure created for a kernel dispatch in SHADOW argument.  */

static void
release_kernel_dispatch (struct GOMP_hsa_kernel_dispatch *shadow)
{
  HSA_DEBUG ("Released kernel dispatch: %p has value: %lu (%p)\n", shadow,
	     shadow->debug, (void *) shadow->debug);

  hsa_fns.hsa_memory_free_fn (shadow->kernarg_address);

  hsa_signal_t s;
  s.handle = shadow->signal;
  hsa_fns.hsa_signal_destroy_fn (s);

  free (shadow->omp_data_memory);

  free (shadow);
}

/* Initialize a KERNEL without its dependencies.  MAX_OMP_DATA_SIZE is used
   to calculate maximum necessary memory for OMP data allocation.  */

static void
init_single_kernel (struct kernel_info *kernel, unsigned *max_omp_data_size)
{
  hsa_status_t status;
  struct agent_info *agent = kernel->agent;
  hsa_executable_symbol_t kernel_symbol;
  status = hsa_fns.hsa_executable_get_symbol_fn (agent->executable, NULL,
						 kernel->name, agent->id,
						 0, &kernel_symbol);
  if (status != HSA_STATUS_SUCCESS)
    {
      hsa_warn ("Could not find symbol for kernel in the code object", status);
      fprintf (stderr, "not found name: '%s'\n", kernel->name);
      dump_executable_symbols (agent->executable);
      goto failure;
    }
  HSA_DEBUG ("Located kernel %s\n", kernel->name);
  status = hsa_fns.hsa_executable_symbol_get_info_fn
    (kernel_symbol, HSA_EXECUTABLE_SYMBOL_INFO_KERNEL_OBJECT, &kernel->object);
  if (status != HSA_STATUS_SUCCESS)
    hsa_fatal ("Could not extract a kernel object from its symbol", status);
  status = hsa_fns.hsa_executable_symbol_get_info_fn
    (kernel_symbol, HSA_EXECUTABLE_SYMBOL_INFO_KERNEL_KERNARG_SEGMENT_SIZE,
     &kernel->kernarg_segment_size);
  if (status != HSA_STATUS_SUCCESS)
    hsa_fatal ("Could not get info about kernel argument size", status);
  status = hsa_fns.hsa_executable_symbol_get_info_fn
    (kernel_symbol, HSA_EXECUTABLE_SYMBOL_INFO_KERNEL_GROUP_SEGMENT_SIZE,
     &kernel->group_segment_size);
  if (status != HSA_STATUS_SUCCESS)
    hsa_fatal ("Could not get info about kernel group segment size", status);
  status = hsa_fns.hsa_executable_symbol_get_info_fn
    (kernel_symbol, HSA_EXECUTABLE_SYMBOL_INFO_KERNEL_PRIVATE_SEGMENT_SIZE,
     &kernel->private_segment_size);
  if (status != HSA_STATUS_SUCCESS)
    hsa_fatal ("Could not get info about kernel private segment size",
	       status);

  HSA_DEBUG ("Kernel structure for %s fully initialized with "
	     "following segment sizes: \n", kernel->name);
  HSA_DEBUG ("  group_segment_size: %u\n",
	     (unsigned) kernel->group_segment_size);
  HSA_DEBUG ("  private_segment_size: %u\n",
	     (unsigned) kernel->private_segment_size);
  HSA_DEBUG ("  kernarg_segment_size: %u\n",
	     (unsigned) kernel->kernarg_segment_size);
  HSA_DEBUG ("  omp_data_size: %u\n", kernel->omp_data_size);
  HSA_DEBUG ("  gridified_kernel_p: %u\n", kernel->gridified_kernel_p);

  if (kernel->omp_data_size > *max_omp_data_size)
    *max_omp_data_size = kernel->omp_data_size;

  return;

failure:
  kernel->initialization_failed = true;
}

/* Indent stream F by INDENT spaces.  */

static void
indent_stream (FILE *f, unsigned indent)
{
  fprintf (f, "%*s", indent, "");
}

/* Dump kernel DISPATCH data structure and indent it by INDENT spaces.  */

static void
print_kernel_dispatch (struct GOMP_hsa_kernel_dispatch *dispatch,
		       unsigned indent)
{
  indent_stream (stderr, indent);
  fprintf (stderr, "this: %p\n", dispatch);
  indent_stream (stderr, indent);
  fprintf (stderr, "queue: %p\n", dispatch->queue);
  indent_stream (stderr, indent);
  fprintf (stderr, "omp_data_memory: %p\n", dispatch->omp_data_memory);
  indent_stream (stderr, indent);
  fprintf (stderr, "kernarg_address: %p\n", dispatch->kernarg_address);
  indent_stream (stderr, indent);
  fprintf (stderr, "object: %lu\n", dispatch->object);
  indent_stream (stderr, indent);
  fprintf (stderr, "signal: %lu\n", dispatch->signal);
  indent_stream (stderr, indent);
  fprintf (stderr, "private_segment_size: %u\n",
	   dispatch->private_segment_size);
  indent_stream (stderr, indent);
  fprintf (stderr, "group_segment_size: %u\n",
	   dispatch->group_segment_size);
  indent_stream (stderr, indent);
  fprintf (stderr, "children dispatches: %lu\n",
	   dispatch->kernel_dispatch_count);
  indent_stream (stderr, indent);
  fprintf (stderr, "omp_num_threads: %u\n",
	   dispatch->omp_num_threads);
  fprintf (stderr, "\n");
}

/* Create kernel dispatch data structure for a KERNEL and all its
   dependencies.  */

static struct GOMP_hsa_kernel_dispatch *
create_kernel_dispatch (struct kernel_info *kernel, unsigned omp_data_size)
{
  struct GOMP_hsa_kernel_dispatch *shadow
    = create_single_kernel_dispatch (kernel, omp_data_size);
  shadow->omp_num_threads = 64;
  shadow->debug = 0;
  shadow->omp_level = kernel->gridified_kernel_p ? 1 : 0;

  return shadow;
}

/* Do all the work that is necessary before running KERNEL for the first time.
   The function assumes the program has been created, finalized and frozen by
   create_and_finalize_hsa_program.  */

static void
init_kernel (struct kernel_info *kernel)
{
  if (pthread_mutex_lock (&kernel->init_mutex))
    GOMP_PLUGIN_fatal ("Could not lock a GCN kernel initialization mutex");
  if (kernel->initialized)
    {
      if (pthread_mutex_unlock (&kernel->init_mutex))
	GOMP_PLUGIN_fatal ("Could not unlock a GCN kernel initialization "
			   "mutex");

      return;
    }

  /* Precomputed maximum size of OMP data necessary for a kernel from kernel
     dispatch operation.  */
  init_single_kernel (kernel, &kernel->max_omp_data_size);

  if (!kernel->initialization_failed)
    {
      HSA_DEBUG ("\n");

      kernel->initialized = true;
    }
  if (pthread_mutex_unlock (&kernel->init_mutex))
    GOMP_PLUGIN_fatal ("Could not unlock a GCN kernel initialization "
		       "mutex");
}

/* Calculate the maximum grid size for OMP threads / OACC workers.
   This depends on the kernel's resource usage levels.  */

static int
limit_worker_threads (int threads)
{
  /* FIXME Do something more inteligent here.
     GCN can always run 4 threads within a Compute Unit, but
     more than that depends on register usage.  */
  if (threads > 16)
    threads = 16;
  return threads;
}

/* Parse the target attributes INPUT provided by the compiler and return true
   if we should run anything all.  If INPUT is NULL, fill DEF with default
   values, then store INPUT or DEF into *RESULT.  */

static bool
parse_target_attributes (void **input,
			 struct GOMP_kernel_launch_attributes *def,
			 struct GOMP_kernel_launch_attributes **result,
			 struct agent_info *agent)
{
  if (!input)
    GOMP_PLUGIN_fatal ("No target arguments provided");

  bool grid_attrs_found = false;
  bool gcn_dims_found = false;
  int gcn_teams = 0;
  int gcn_threads = 0;
  while (*input)
    {
      intptr_t id = (intptr_t) *input++, val;

      if (id & GOMP_TARGET_ARG_SUBSEQUENT_PARAM)
	val = (intptr_t) *input++;
      else
	val = id >> GOMP_TARGET_ARG_VALUE_SHIFT;

      val = (val > INT_MAX) ? INT_MAX : val;

      if ((id & GOMP_TARGET_ARG_DEVICE_MASK) == GOMP_DEVICE_GCN
	  && ((id & GOMP_TARGET_ARG_ID_MASK)
	      == GOMP_TARGET_ARG_HSA_KERNEL_ATTRIBUTES))
	{
	  grid_attrs_found = true;
	  break;
	}
      else if ((id & GOMP_TARGET_ARG_DEVICE_ALL) == GOMP_TARGET_ARG_DEVICE_ALL)
	{
	  gcn_dims_found = true;
	  switch (id & GOMP_TARGET_ARG_ID_MASK)
	    {
	    case GOMP_TARGET_ARG_NUM_TEAMS:
	      gcn_teams = val;
	      break;
	    case GOMP_TARGET_ARG_THREAD_LIMIT:
	      gcn_threads = limit_worker_threads (val);
	      break;
	    default:
	      ;
	    }
	}
    }

  if (gcn_dims_found)
    {
      if (agent->gfx900_p && gcn_threads == 0 && override_z_dim == 0)
	{
	  gcn_threads = 4;
	  HSA_DEBUG ("VEGA BUG WORKAROUND: reducing default number of "
		     "threads to 4 per team.\n");
	  HSA_DEBUG (" - If this is not a Vega 10 device, please use "
		     "GCN_NUM_THREADS=16\n");
	}

      def->ndim = 3;
      /* Fiji has 64 CUs.  */
      def->gdims[0] = (gcn_teams > 0) ? gcn_teams : 64;
      /* Each thread is 64 work items wide.  */
      def->gdims[1] = 64;
      /* A work group can have 16 wavefronts.  */
      def->gdims[2] = (gcn_threads > 0) ? gcn_threads : 16;
      def->wdims[0] = 1; /* Single team per work-group.  */
      def->wdims[1] = 64;
      def->wdims[2] = 16;
      *result = def;
      return true;
    }
  else if (!grid_attrs_found)
    {
      def->ndim = 1;
      def->gdims[0] = 1;
      def->gdims[1] = 1;
      def->gdims[2] = 1;
      def->wdims[0] = 1;
      def->wdims[1] = 1;
      def->wdims[2] = 1;
      *result = def;
      HSA_DEBUG ("GOMP_OFFLOAD_run called with no launch attributes\n");
      return true;
    }

  struct GOMP_kernel_launch_attributes *kla;
  kla = (struct GOMP_kernel_launch_attributes *) *input;
  *result = kla;
  if (kla->ndim == 0 || kla->ndim > 3)
    GOMP_PLUGIN_fatal ("Invalid number of dimensions (%u)", kla->ndim);

  HSA_DEBUG ("GOMP_OFFLOAD_run called with %u dimensions:\n", kla->ndim);
  unsigned i;
  for (i = 0; i < kla->ndim; i++)
    {
      HSA_DEBUG ("  Dimension %u: grid size %u and group size %u\n", i,
		 kla->gdims[i], kla->wdims[i]);
      if (kla->gdims[i] == 0)
	return false;
    }
  return true;
}

/* Return the group size given the requested GROUP size, GRID size and number
   of grid dimensions NDIM.  */

static uint32_t
get_group_size (uint32_t ndim, uint32_t grid, uint32_t group)
{
  if (group == 0)
    {
      /* TODO: Provide a default via environment or device characteristics.  */
      if (ndim == 1)
	group = 64;
      else if (ndim == 2)
	group = 8;
      else
	group = 4;
    }

  if (group > grid)
    group = grid;
  return group;
}

/* Return true if the HSA runtime can run function FN_PTR.  */

bool
GOMP_OFFLOAD_can_run (void *fn_ptr)
{
  struct kernel_info *kernel = (struct kernel_info *) fn_ptr;

  init_kernel (kernel);
  if (kernel->initialization_failed)
    goto failure;

  return true;

failure:
  if (suppress_host_fallback)
    GOMP_PLUGIN_fatal ("GCN host fallback has been suppressed");
  HSA_DEBUG ("GCN target cannot be launched, doing a host fallback\n");
  return false;
}

/* Atomically store pair of uint16_t values (HEADER and REST) to a PACKET.  */

void
packet_store_release (uint32_t* packet, uint16_t header, uint16_t rest)
{
  __atomic_store_n (packet, header | (rest << 16), __ATOMIC_RELEASE);
}

/* Run KERNEL on its agent, pass VARS to it as arguments and take
   launchattributes from KLA.  MODULE_LOCKED indicates that the caller
   already holds the lock and run_kernel need not lock it again.
   If AQ is NULL then agent->sync_queue will be used.  */

static void
run_kernel (struct kernel_info *kernel, void *vars,
	    struct GOMP_kernel_launch_attributes *kla,
	    struct goacc_asyncqueue *aq, bool module_locked)
{
  HSA_DEBUG ("GCN launch on queue: %d:%d\n", kernel->agent->device_id,
	     (aq ? aq->id : 0));
  HSA_DEBUG ("GCN launch attribs: gdims:[");
  int i;
  for (i = 0; i < kla->ndim; ++i)
    {
      if (i)
	HSA_DPRINT (", ");
      HSA_DPRINT ("%u", kla->gdims[i]);
    }
  HSA_DPRINT ("], normalized gdims:[");
  for (i = 0; i < kla->ndim; ++i)
    {
      if (i)
	HSA_DPRINT (", ");
      HSA_DPRINT ("%u", kla->gdims[i] / kla->wdims[i]);
    }
  HSA_DPRINT ("], wdims:[");
  for (i = 0; i < kla->ndim; ++i)
    {
      if (i)
	HSA_DPRINT (", ");
      HSA_DPRINT ("%u", kla->wdims[i]);
    }
  HSA_DPRINT ("]\n");
  HSA_FLUSH ();

  struct agent_info *agent = kernel->agent;
  if (!module_locked && pthread_rwlock_rdlock (&agent->module_rwlock))
    GOMP_PLUGIN_fatal ("Unable to read-lock a GCN agent rwlock");

  if (!agent->initialized)
    GOMP_PLUGIN_fatal ("Agent must be initialized");

  if (!kernel->initialized)
    GOMP_PLUGIN_fatal ("Called kernel must be initialized");

  struct GOMP_hsa_kernel_dispatch *shadow
    = create_kernel_dispatch (kernel, kernel->max_omp_data_size);

  hsa_queue_t *command_q = (aq ? aq->hsa_queue : kernel->agent->sync_queue);
  shadow->queue = command_q;

  if (debug)
    {
      fprintf (stderr, "\nKernel has following dependencies:\n");
      print_kernel_dispatch (shadow, 2);
    }

  uint64_t index
    = hsa_fns.hsa_queue_add_write_index_release_fn (command_q, 1);
  HSA_DEBUG ("Got AQL index %llu\n", (long long int) index);

  /* Wait until the queue is not full before writing the packet.   */
  while (index - hsa_fns.hsa_queue_load_read_index_acquire_fn (command_q)
	 >= command_q->size)
    ;

  /* Do not allow the dimensions to be overridden when running
     constructors or destructors.  */
  struct module_info *module = kernel->module;
  bool init_fini_p = kernel == module->init_array_func
		     || kernel == module->fini_array_func;
  int override_x = init_fini_p ? 0 : override_x_dim;
  int override_z = init_fini_p ? 0 : override_z_dim;

  hsa_kernel_dispatch_packet_t *packet;
  packet = ((hsa_kernel_dispatch_packet_t *) command_q->base_address)
	   + index % command_q->size;

  memset (((uint8_t *) packet) + 4, 0, sizeof (*packet) - 4);
  packet->grid_size_x = override_x ? : kla->gdims[0];
  packet->workgroup_size_x = get_group_size (kla->ndim,
					     packet->grid_size_x,
					     kla->wdims[0]);

  if (kla->ndim >= 2)
    {
      packet->grid_size_y = kla->gdims[1];
      packet->workgroup_size_y = get_group_size (kla->ndim, kla->gdims[1],
						 kla->wdims[1]);
    }
  else
    {
      packet->grid_size_y = 1;
      packet->workgroup_size_y = 1;
    }

  if (kla->ndim == 3)
    {
      packet->grid_size_z = limit_worker_threads (override_z
						  ? : kla->gdims[2]);
      packet->workgroup_size_z = get_group_size (kla->ndim,
						 packet->grid_size_z,
						 kla->wdims[2]);
    }
  else
    {
      packet->grid_size_z = 1;
      packet->workgroup_size_z = 1;
    }

  HSA_DEBUG ("GCN launch actuals: grid:[%u, %u, %u],"
	     " normalized grid:[%u, %u, %u], workgroup:[%u, %u, %u]\n",
	     packet->grid_size_x, packet->grid_size_y, packet->grid_size_z,
	     packet->grid_size_x / packet->workgroup_size_x,
	     packet->grid_size_y / packet->workgroup_size_y,
	     packet->grid_size_z / packet->workgroup_size_z,
	     packet->workgroup_size_x, packet->workgroup_size_y,
	     packet->workgroup_size_z);

  packet->private_segment_size = kernel->private_segment_size;
  packet->group_segment_size = kernel->group_segment_size;
  packet->kernel_object = kernel->object;
  packet->kernarg_address = shadow->kernarg_address;
  hsa_signal_t s;
  s.handle = shadow->signal;
  packet->completion_signal = s;
  hsa_fns.hsa_signal_store_relaxed_fn (s, 1);
  memcpy (shadow->kernarg_address, &vars, sizeof (vars));

  /* PR hsa/70337.  */
  size_t vars_size = sizeof (vars);
  if (kernel->kernarg_segment_size > vars_size)
    {
      if (kernel->kernarg_segment_size != vars_size
	  + sizeof (struct hsa_kernel_runtime *))
	GOMP_PLUGIN_fatal ("Kernel segment size has an unexpected value");
      memcpy (packet->kernarg_address + vars_size, &shadow,
	      sizeof (struct hsa_kernel_runtime *));
    }

  HSA_DEBUG ("Copying kernel runtime pointer to kernarg_address\n");

  uint16_t header;
  header = HSA_PACKET_TYPE_KERNEL_DISPATCH << HSA_PACKET_HEADER_TYPE;
  header |= HSA_FENCE_SCOPE_SYSTEM << HSA_PACKET_HEADER_ACQUIRE_FENCE_SCOPE;
  header |= HSA_FENCE_SCOPE_SYSTEM << HSA_PACKET_HEADER_RELEASE_FENCE_SCOPE;

  HSA_DEBUG ("Going to dispatch kernel %s on device %d\n", kernel->name,
	     agent->device_id);

  packet_store_release ((uint32_t *) packet, header,
			(uint16_t) kla->ndim
			<< HSA_KERNEL_DISPATCH_PACKET_SETUP_DIMENSIONS);

  hsa_fns.hsa_signal_store_release_fn (command_q->doorbell_signal,
				       index);

  HSA_DEBUG ("Kernel dispatched, waiting for completion\n");

  /* Root signal waits with 1ms timeout.  */
  while (hsa_fns.hsa_signal_wait_acquire_fn (s, HSA_SIGNAL_CONDITION_LT, 1,
					     1000 * 1000,
					     HSA_WAIT_STATE_BLOCKED) != 0)
    {
      gomp_print_output (kernel, shadow->kernarg_address, false);
    }
  gomp_print_output (kernel, shadow->kernarg_address, true);

  struct kernargs *kernargs = shadow->kernarg_address;
  unsigned int return_value = (unsigned int)kernargs->output_data.return_value;

  release_kernel_dispatch (shadow);

  if (!module_locked && pthread_rwlock_unlock (&agent->module_rwlock))
    GOMP_PLUGIN_fatal ("Unable to unlock a GCN agent rwlock");

  unsigned int upper = (return_value & ~0xffff) >> 16;
  if (upper == 0xcafe)
    ; // exit not called, normal termination.
  else if (upper == 0xffff)
    ; // exit called.
  else
    {
      GOMP_PLUGIN_error ("Possible kernel exit value corruption, 2 most"
			 " significant bytes aren't 0xffff or 0xcafe: 0x%x\n",
			 return_value);
      abort ();
    }

  if (upper == 0xffff)
    {
      unsigned int signal = (return_value >> 8) & 0xff;

      if (signal == SIGABRT)
	{
	  HSA_DEBUG ("GCN Kernel aborted\n");
	  abort ();
	}
      else if (signal != 0)
	{
	  HSA_DEBUG ("GCN Kernel received unknown signal\n");
	  abort ();
	}

      HSA_DEBUG ("GCN Kernel exited with value: %d\n", return_value & 0xff);
      exit (return_value & 0xff);
    }
}

/* Part of the libgomp plugin interface.  Run a kernel on device N (the number
   is actually ignored, we assume the FN_PTR has been mapped using the correct
   device) and pass it an array of pointers in VARS as a parameter.  The kernel
   is identified by FN_PTR which must point to a kernel_info structure.  */

void
GOMP_OFFLOAD_run (int device, void *fn_ptr, void *vars, void **args)
{
  struct agent_info *agent = get_agent_info (device);
  struct kernel_info *kernel = (struct kernel_info *) fn_ptr;
  struct GOMP_kernel_launch_attributes def;
  struct GOMP_kernel_launch_attributes *kla;
  assert (agent == kernel->agent);

  if (!parse_target_attributes (args, &def, &kla, agent))
    {
      HSA_DEBUG ("Will not run GCN kernel because the grid size is zero\n");
      return;
    }
  run_kernel (kernel, vars, kla, NULL, false);
}

/* Set up an async queue for OpenMP.  There will be only one.
   FIXME: is this thread-safe if two threads call this function?  */
static void
maybe_init_omp_async (struct agent_info *agent)
{
  if (!agent->omp_async_queue)
    agent->omp_async_queue
      = GOMP_OFFLOAD_openacc_async_construct (agent->device_id);
}

/* Part of the libgomp plugin interface.  Run a kernel like GOMP_OFFLOAD_run
   does, but asynchronously and call GOMP_PLUGIN_target_task_completion when it
   has finished.  */

void
GOMP_OFFLOAD_async_run (int device, void *tgt_fn, void *tgt_vars,
			void **args, void *async_data)
{
  HSA_DEBUG ("GOMP_OFFLOAD_async_run invoked\n");
  struct agent_info *agent = get_agent_info (device);
  struct kernel_info *kernel = (struct kernel_info *) tgt_fn;
  struct GOMP_kernel_launch_attributes def;
  struct GOMP_kernel_launch_attributes *kla;
  assert (agent == kernel->agent);

  if (!parse_target_attributes (args, &def, &kla, agent))
    {
      HSA_DEBUG ("Will not run GCN kernel because the grid size is zero\n");
      return;
    }

  maybe_init_omp_async (agent);
  queue_push_launch (agent->omp_async_queue, kernel, tgt_vars, kla);
  queue_push_callback (agent->omp_async_queue,
		       GOMP_PLUGIN_target_task_completion, async_data);
}

/* Deinitialize all information associated with MODULE and kernels within
   it.  Return TRUE on success.  */

static bool
destroy_module (struct module_info *module, bool locked)
{
  /* Run destructors before destroying module.  */
  struct GOMP_kernel_launch_attributes kla =
    { 3,
      /* Grid size.  */
      { 1, 64, 1 },
      /* Work-group size.  */
      { 1, 64, 1 }
    };

  if (module->fini_array_func)
    {
      init_kernel (module->fini_array_func);
      run_kernel (module->fini_array_func, NULL, &kla, NULL, locked);
    }
  module->constructors_run_p = false;

  int i;
  for (i = 0; i < module->kernel_count; i++)
    if (pthread_mutex_destroy (&module->kernels[i].init_mutex))
      {
	GOMP_PLUGIN_error ("Failed to destroy a GCN kernel initialization "
			   "mutex");
	return false;
      }

  return true;
}

/* Part of the libgomp plugin interface.  Unload GCN object-code module
   described by struct gcn_image_desc in TARGET_DATA from agent number N.
   Return TRUE on success.  */

bool
GOMP_OFFLOAD_unload_image (int n, unsigned version, const void *target_data)
{
  if (GOMP_VERSION_DEV (version) > GOMP_VERSION_HSA)
    {
      GOMP_PLUGIN_error ("Offload data incompatible with GCN plugin"
			 " (expected %u, received %u)",
			 GOMP_VERSION_GCN, GOMP_VERSION_DEV (version));
      return false;
    }

  struct agent_info *agent;
  agent = get_agent_info (n);
  if (!agent)
    return false;

  if (pthread_rwlock_wrlock (&agent->module_rwlock))
    {
      GOMP_PLUGIN_error ("Unable to write-lock a GCN agent rwlock");
      return false;
    }

  if (!agent->module || agent->module->image_desc != target_data)
    {
      GOMP_PLUGIN_error ("Attempt to unload an image that has never been "
			 "loaded before");
      return false;
    }

  if (!destroy_module (agent->module, true))
    return false;
  free (agent->module);
  agent->module = NULL;
  if (!destroy_hsa_program (agent))
    return false;
  if (pthread_rwlock_unlock (&agent->module_rwlock))
    {
      GOMP_PLUGIN_error ("Unable to unlock a GCN agent rwlock");
      return false;
    }
  return true;
}

/* Part of the libgomp plugin interface.  Deinitialize all information and
   status associated with agent number N.  We do not attempt any
   synchronization, assuming the user and libgomp will not attempt
   deinitialization of a device that is in any way being used at the same
   time.  Return TRUE on success.  */

bool
GOMP_OFFLOAD_fini_device (int n)
{
  struct agent_info *agent = get_agent_info (n);
  if (!agent)
    return false;

  if (!agent->initialized)
    return true;

  if (agent->omp_async_queue)
    {
      GOMP_OFFLOAD_openacc_async_destruct (agent->omp_async_queue);
      agent->omp_async_queue = NULL;
    }

  if (agent->module)
    {
      if (!destroy_module (agent->module, false))
        return false;
      free (agent->module);
      agent->module = NULL;
    }

  if (!destroy_hsa_program (agent))
    return false;

  /*release_agent_shared_libraries (agent);*/

  hsa_status_t status = hsa_fns.hsa_queue_destroy_fn (agent->sync_queue);
  if (status != HSA_STATUS_SUCCESS)
    return hsa_error ("Error destroying command queue", status);

  if (pthread_mutex_destroy (&agent->prog_mutex))
    {
      GOMP_PLUGIN_error ("Failed to destroy a GCN agent program mutex");
      return false;
    }
  if (pthread_rwlock_destroy (&agent->module_rwlock))
    {
      GOMP_PLUGIN_error ("Failed to destroy a GCN agent rwlock");
      return false;
    }

  if (pthread_mutex_destroy (&agent->async_queues_mutex))
    {
      GOMP_PLUGIN_error ("Failed to destroy a GCN agent queue mutex");
      return false;
    }
  agent->initialized = false;
  return true;
}

static void *
GOMP_OFFLOAD_alloc_by_agent (struct agent_info *agent, size_t size)
{
  HSA_DEBUG ("Allocating %zu bytes on device %d\n", size, agent->device_id);

  /* Zero-size allocations are invalid, so in order to return a valid pointer
     we need to pass a valid size.  One source of zero-size allocations is
     kernargs for kernels that have no inputs or outputs (the kernel may
     only use gomp_print, for example).  */
  if (size == 0)
    size = 4;

  void *ptr;
  hsa_status_t status = hsa_fns.hsa_memory_allocate_fn (agent->kernarg_region,
							size, &ptr);
  if (status != HSA_STATUS_SUCCESS)
    {
      hsa_error ("Could not allocate device memory", status);
      return NULL;
    }

  return ptr;
}

void *
GOMP_OFFLOAD_alloc (int n, size_t size)
{
  struct agent_info *agent = get_agent_info (n);
  return GOMP_OFFLOAD_alloc_by_agent (agent, size);
}

bool
GOMP_OFFLOAD_free (int device, void *ptr)
{
  HSA_DEBUG ("Freeing memory on device %d\n", device);

  hsa_status_t status = hsa_fns.hsa_memory_free_fn (ptr);
  if (status != HSA_STATUS_SUCCESS)
    {
      hsa_error ("Could not free device memory", status);
      return false;
    }

  return true;
}

/* Returns true if PTR falls within the bounds of any loaded kernel image.  */

static bool
image_address_p (struct agent_info *agent, const void *ptr)
{
  Elf64_Addr addr = (Elf64_Addr)ptr;
  if (agent->module)
    {
      if (addr >= agent->module->phys_address_start
	  && addr <= agent->module->phys_address_end)
	return true;
    }
  return false;
}

struct copy_data
{
  void *dst;
  const void *src;
  size_t len;
  bool use_hsa_memory_copy;
  struct goacc_asyncqueue *aq;
};

static void
copy_data (void *data_)
{
  struct copy_data *data = (struct copy_data *)data_;
  HSA_DEBUG ("Async thread %d:%d: Copying %zu bytes from (%p) to (%p)\n",
	     data->aq->agent->device_id, data->aq->id, data->len, data->src,
	     data->dst);
  if (data->use_hsa_memory_copy)
    hsa_fns.hsa_memory_copy_fn (data->dst, data->src, data->len);
  else
    memcpy (data->dst, data->src, data->len);
  free (data);
}

static void
queue_push_copy (struct goacc_asyncqueue *aq, void *dst, const void *src,
		 size_t len, bool use_hsa_memory_copy)
{
  if (DEBUG_QUEUES)
    HSA_DEBUG ("queue_push_copy %d:%d: %zu bytes from (%p) to (%p)\n",
	       aq->agent->device_id, aq->id, len, src, dst);
  struct copy_data *data
    = (struct copy_data *)GOMP_PLUGIN_malloc (sizeof (struct copy_data));
  data->dst = dst;
  data->src = src;
  data->len = len;
  data->use_hsa_memory_copy = use_hsa_memory_copy;
  data->aq = aq;
  queue_push_callback (aq, copy_data, data);
}

bool
GOMP_OFFLOAD_dev2host (int device, void *dst, const void *src, size_t n)
{
  HSA_DEBUG ("Copying %zu bytes from device %d (%p) to host (%p)\n", n, device,
	     src, dst);

  /* memcpy only works for addresses allocated with hsa_memory_allocate,
     but hsa_memory_copy seems unable to read from .rodata variables.  */
  if (image_address_p (get_agent_info (device), src))
    hsa_fns.hsa_memory_copy_fn (dst, src, n);
  else
    memcpy (dst, src, n);
  return true;
}

bool
GOMP_OFFLOAD_host2dev (int device, void *dst, const void *src, size_t n)
{
  HSA_DEBUG ("Copying %zu bytes from host (%p) to device %d (%p)\n", n, src,
	     device, dst);
  /* memcpy only works for addresses allocated with hsa_memory_allocate,
     but hsa_memory_copy seems unable to read from .rodata variables.  */
  if (image_address_p (get_agent_info (device), dst))
    hsa_fns.hsa_memory_copy_fn (dst, src, n);
  else
    memcpy (dst, src, n);
  return true;
}

/* Part of the libgomp plugin interface.  */

bool
GOMP_OFFLOAD_dev2dev (int device, void *dst, const void *src, size_t n)
{
  struct gcn_thread *thread_data = gcn_thread ();

  if (thread_data && !async_synchronous_p (thread_data->async))
    {
      struct agent_info *agent = get_agent_info (device);
      maybe_init_omp_async (agent);
      queue_push_copy (agent->omp_async_queue, dst, src, n, false);
      return true;
    }

  HSA_DEBUG ("Copying %zu bytes from device %d (%p) to device %d (%p)\n", n,
	     device, src, device, dst);
  /* We can assume that dev2dev moves are always within allocated memory.  */
  memcpy (dst, src, n);
  return true;
}

static int
queue_empty (struct goacc_asyncqueue *aq)
{
  pthread_mutex_lock (&aq->mutex);
  int res = aq->queue_n == 0 ? 1 : 0;
  pthread_mutex_unlock (&aq->mutex);

  return res;
}

static void
wait_queue (struct goacc_asyncqueue *aq)
{
  if (DRAIN_QUEUE_SYNCHRONOUS_P)
    {
      drain_queue_synchronous (aq);
      return;
    }

  pthread_mutex_lock (&aq->mutex);

  while (aq->queue_n > 0)
    {
      if (DEBUG_THREAD_SLEEP)
	HSA_DEBUG ("waiting for thread %d:%d, putting thread to sleep\n",
		   aq->agent->device_id, aq->id);
      pthread_cond_wait (&aq->queue_cond_out, &aq->mutex);
      if (DEBUG_THREAD_SLEEP)
	HSA_DEBUG ("thread %d:%d woke up.  Rechecking\n", aq->agent->device_id,
		   aq->id);
    }

  pthread_mutex_unlock (&aq->mutex);
  HSA_DEBUG ("waiting for thread %d:%d, done\n", aq->agent->device_id, aq->id);
}

static void
gomp_offload_free (void *ptr)
{
  HSA_DEBUG ("Async thread ?:?: Freeing %p\n", ptr);
  GOMP_OFFLOAD_free (0, ptr);
}

static void
gcn_exec (struct kernel_info *kernel, size_t mapnum, void **hostaddrs,
	  void **devaddrs, unsigned *dims, void *targ_mem_desc, bool async,
	  struct goacc_asyncqueue *aq)
{
  if (!GOMP_OFFLOAD_can_run (kernel))
    GOMP_PLUGIN_fatal ("OpenACC host fallback unimplemented.");

  // For some reason, devaddrs must be double-indirect on the target
  void **ind_da = GOMP_OFFLOAD_alloc_by_agent (kernel->agent,
					       sizeof (void*) * mapnum);
  for (size_t i = 0; i < mapnum; i++)
    ind_da[i] = devaddrs[i] ? devaddrs[i] : hostaddrs[i];

  struct hsa_kernel_description *hsa_kernel_desc = NULL;
  for (unsigned i = 0; i < kernel->module->image_desc->kernel_count; i++)
    {
      struct hsa_kernel_description *d
	= &kernel->module->image_desc->kernel_infos[i];
      if (d->name == kernel->name)
	{
	  hsa_kernel_desc = d;
	  break;
	}
    }

  /* We may have statically-determined dimensions in
     hsa_kernel_desc->oacc_dims[] or dimensions passed to this offload kernel
     invocation at runtime in dims[].  We allow static dimensions to take
     priority over dynamic dimensions when present (non-zero).  */
  if (hsa_kernel_desc->oacc_dims[0] > 0)
    dims[0] = hsa_kernel_desc->oacc_dims[0];
  if (hsa_kernel_desc->oacc_dims[1] > 0)
    dims[1] = hsa_kernel_desc->oacc_dims[1];
  if (hsa_kernel_desc->oacc_dims[2] > 0)
    dims[2] = hsa_kernel_desc->oacc_dims[2];

  /* If any of the OpenACC dimensions remain 0 then we get to pick a number.
     There isn't really a correct answer for this without a clue about the
     problem size, so let's do a reasonable number of single-worker gangs.
     64 gangs matches a typical Fiji device.  */

  if (dims[0] == 0) dims[0] = 64; /* Gangs.  */
  if (dims[1] == 0) dims[1] = 16; /* Workers.  */

  /* The incoming dimensions are expressed in terms of gangs, workers, and
     vectors.  The HSA dimensions are expressed in terms of "work-items",
     which means multiples of vector lanes.

     The "grid size" specifies the size of the problem space, and the
     "work-group size" specifies how much of that we want a single compute
     unit to chew on at once.

     The three dimensions do not really correspond to hardware, but the
     important thing is that the HSA runtime will launch as many
     work-groups as it takes to process the entire grid, and each
     work-group will contain as many wave-fronts as it takes to process
     the work-items in that group.

     Essentially, as long as we set the Y dimension to 64 (the number of
     vector lanes in hardware), and the Z group size to the maximum (16),
     then we will get the gangs (X) and workers (Z) launched as we expect.

     The reason for the apparent reversal of vector and worker dimension
     order is to do with the way the run-time distributes work-items across
     v1 and v2.  */
  struct GOMP_kernel_launch_attributes kla =
    {3,
     /* Grid size.  */
     {dims[0], 64, dims[1]},
     /* Work-group size.  */
     {1,       64, 16}
    };

  if (!async)
    {
      run_kernel (kernel, ind_da, &kla, NULL, false);
      gomp_offload_free (ind_da);
    }
  else
    {
      queue_push_launch (aq, kernel, ind_da, &kla);
      if (DEBUG_QUEUES)
	HSA_DEBUG ("queue_push_callback %d:%d gomp_offload_free, %p\n",
		   aq->agent->device_id, aq->id, ind_da);
      queue_push_callback (aq, gomp_offload_free, ind_da);
    }
}

void
GOMP_OFFLOAD_openacc_exec (void (*fn_ptr) (void *), size_t mapnum,
			   void **hostaddrs, void **devaddrs, unsigned *dims,
			   void *targ_mem_desc)
{
  struct kernel_info *kernel = (struct kernel_info *) fn_ptr;

  gcn_exec (kernel, mapnum, hostaddrs, devaddrs, dims, targ_mem_desc, false,
	    NULL);
}

void
GOMP_OFFLOAD_openacc_exec_params (void (*fn_ptr) (void *), size_t mapnum,
				  void **hostaddrs, void **devaddrs,
				  unsigned *dims, void *targ_mem_desc)
{
  GOMP_PLUGIN_fatal ("OpenACC exec params unimplemented.");
}

void
GOMP_OFFLOAD_openacc_async_exec (void (*fn_ptr) (void *), size_t mapnum,
				 void **hostaddrs, void **devaddrs,
				 unsigned *dims, void *targ_mem_desc,
				 struct goacc_asyncqueue *aq)
{
  struct kernel_info *kernel = (struct kernel_info *) fn_ptr;

  gcn_exec (kernel, mapnum, hostaddrs, devaddrs, dims, targ_mem_desc, true,
	    aq);
}

void
GOMP_OFFLOAD_openacc_async_exec_params (void (*fn) (void *), size_t mapnum,
					void **hostaddrs, void **devaddrs,
					unsigned *dims, void *targ_mem_desc,
					struct goacc_asyncqueue *aq)
{
  GOMP_PLUGIN_fatal ("OpenACC async exec params unimplemented.");
}

struct goacc_asyncqueue *
GOMP_OFFLOAD_openacc_async_construct (int device)
{
  struct agent_info *agent = get_agent_info (device);

  pthread_mutex_lock (&agent->async_queues_mutex);

  struct goacc_asyncqueue *aq = GOMP_PLUGIN_malloc (sizeof (*aq));
  aq->agent = get_agent_info (device);
  aq->prev = NULL;
  aq->next = agent->async_queues;
  if (aq->next)
    {
      aq->next->prev = aq;
      aq->id = aq->next->id + 1;
    }
  else
    aq->id = 1;
  agent->async_queues = aq;

  aq->queue_first = 0;
  aq->queue_n = 0;
  aq->drain_queue_stop = 0;

  if (pthread_mutex_init (&aq->mutex, NULL))
    {
      GOMP_PLUGIN_error ("Failed to initialize a GCN agent queue mutex");
      return false;
    }
  if (pthread_cond_init (&aq->queue_cond_in, NULL))
    {
      GOMP_PLUGIN_error ("Failed to initialize a GCN agent queue cond");
      return false;
    }
  if (pthread_cond_init (&aq->queue_cond_out, NULL))
    {
      GOMP_PLUGIN_error ("Failed to initialize a GCN agent queue cond");
      return false;
    }

  hsa_status_t status = hsa_fns.hsa_queue_create_fn (agent->id,
						     ASYNC_QUEUE_SIZE,
						     HSA_QUEUE_TYPE_MULTI,
						     queue_callback, NULL,
						     UINT32_MAX, UINT32_MAX,
						     &aq->hsa_queue);
  if (status != HSA_STATUS_SUCCESS)
    hsa_fatal ("Error creating command queue", status);

  int err = pthread_create (&aq->thread_drain_queue, NULL, &drain_queue, aq);
  if (err != 0)
    GOMP_PLUGIN_fatal ("GCN asynchronous thread creation failed: %s",
		       strerror (err));
  HSA_DEBUG ("Async thread %d:%d: created\n", aq->agent->device_id,
	     aq->id);

  pthread_mutex_unlock (&agent->async_queues_mutex);

  return aq;
}

bool
GOMP_OFFLOAD_openacc_async_destruct (struct goacc_asyncqueue *aq)
{
  struct agent_info *agent = aq->agent;

  finalize_async_thread (aq);

  pthread_mutex_lock (&agent->async_queues_mutex);

  int err;
  if ((err = pthread_mutex_destroy (&aq->mutex)))
    {
      GOMP_PLUGIN_error ("Failed to destroy a GCN async queue mutex: %d", err);
      goto fail;
    }
  if (pthread_cond_destroy (&aq->queue_cond_in))
    {
      GOMP_PLUGIN_error ("Failed to destroy a GCN async queue cond");
      goto fail;
    }
  if (pthread_cond_destroy (&aq->queue_cond_out))
    {
      GOMP_PLUGIN_error ("Failed to destroy a GCN async queue cond");
      goto fail;
    }
  hsa_status_t status = hsa_fns.hsa_queue_destroy_fn (aq->hsa_queue);
  if (status != HSA_STATUS_SUCCESS)
    {
      hsa_error ("Error destroying command queue", status);
      goto fail;
    }

  if (aq->prev)
    aq->prev->next = aq->next;
  if (aq->next)
    aq->next->prev = aq->prev;
  if (agent->async_queues == aq)
    agent->async_queues = aq->next;

  HSA_DEBUG ("Async thread %d:%d: destroyed\n", agent->device_id, aq->id);

  free (aq);
  pthread_mutex_unlock (&agent->async_queues_mutex);
  return true;

fail:
  pthread_mutex_unlock (&agent->async_queues_mutex);
  return false;
}

int
GOMP_OFFLOAD_openacc_async_test (struct goacc_asyncqueue *aq)
{
  return queue_empty (aq);
}

bool
GOMP_OFFLOAD_openacc_async_synchronize (struct goacc_asyncqueue *aq)
{
  wait_queue (aq);
  return true;
}

bool
GOMP_OFFLOAD_openacc_async_serialize (struct goacc_asyncqueue *aq1,
				      struct goacc_asyncqueue *aq2)
{
  /* FIXME: what should happen here????  */
  wait_queue (aq1);
  wait_queue (aq2);
  return true;
}

void
GOMP_OFFLOAD_openacc_async_queue_callback (struct goacc_asyncqueue *aq,
					   void (*fn) (void *), void *data)
{
  queue_push_callback (aq, fn, data);
}

bool
GOMP_OFFLOAD_openacc_async_host2dev (int device, void *dst, const void *src,
				     size_t n, struct goacc_asyncqueue *aq)
{
  struct agent_info *agent = get_agent_info (device);
  assert (agent == aq->agent);
  queue_push_copy (aq, dst, src, n, image_address_p (agent, dst));
  return true;
}

bool
GOMP_OFFLOAD_openacc_async_dev2host (int device, void *dst, const void *src,
				     size_t n, struct goacc_asyncqueue *aq)
{
  struct agent_info *agent = get_agent_info (device);
  assert (agent == aq->agent);
  queue_push_copy (aq, dst, src, n, image_address_p (agent, src));
  return true;
}

void *
GOMP_OFFLOAD_openacc_create_thread_data (int ord __attribute__((unused)))
{
  struct gcn_thread *thread_data
    = GOMP_PLUGIN_malloc (sizeof (struct gcn_thread));

  thread_data->async = GOMP_ASYNC_SYNC;

  return (void *) thread_data;
}

void
GOMP_OFFLOAD_openacc_destroy_thread_data (void *data)
{
  free (data);
}
