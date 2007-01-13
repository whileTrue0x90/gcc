
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_javax_crypto_sasl_srp_SRPRegistry__
#define __gnu_javax_crypto_sasl_srp_SRPRegistry__

#pragma interface

#include <java/lang/Object.h>
#include <gcj/array.h>

extern "Java"
{
  namespace gnu
  {
    namespace javax
    {
      namespace crypto
      {
        namespace sasl
        {
          namespace srp
          {
              class SRPRegistry;
          }
        }
      }
    }
  }
}

class gnu::javax::crypto::sasl::srp::SRPRegistry : public ::java::lang::Object
{

public:
  static ::java::lang::String * N_2048_BITS;
  static ::java::lang::String * N_1536_BITS;
  static ::java::lang::String * N_1280_BITS;
  static ::java::lang::String * N_1024_BITS;
  static ::java::lang::String * N_768_BITS;
  static ::java::lang::String * N_640_BITS;
  static ::java::lang::String * N_512_BITS;
  static JArray< ::java::lang::String * > * SRP_ALGORITHMS;
  static ::java::lang::String * SRP_DEFAULT_DIGEST_NAME;
  static ::java::lang::String * SRP_DIGEST_NAME;
  static ::java::lang::String * SHARED_MODULUS;
  static ::java::lang::String * FIELD_GENERATOR;
  static ::java::lang::String * AVAILABLE_OPTIONS;
  static ::java::lang::String * CHOSEN_OPTIONS;
  static ::java::lang::String * USER_NAME;
  static ::java::lang::String * USER_ROLE;
  static ::java::lang::String * USER_SALT;
  static ::java::lang::String * PASSWORD_VERIFIER;
  static ::java::lang::String * CLIENT_PUBLIC_KEY;
  static ::java::lang::String * SERVER_PUBLIC_KEY;
  static ::java::lang::String * CLIENT_EVIDENCE;
  static ::java::lang::String * SERVER_EVIDENCE;
  static ::java::lang::String * SRP_HASH;
  static ::java::lang::String * SRP_MANDATORY;
  static ::java::lang::String * SRP_REPLAY_DETECTION;
  static ::java::lang::String * SRP_INTEGRITY_PROTECTION;
  static ::java::lang::String * SRP_CONFIDENTIALITY;
  static ::java::lang::String * PASSWORD_FILE;
  static ::java::lang::String * PASSWORD_DB;
  static ::java::lang::String * DEFAULT_PASSWORD_FILE;
  static const jboolean DEFAULT_REPLAY_DETECTION = 1;
  static const jboolean DEFAULT_INTEGRITY = 1;
  static const jboolean DEFAULT_CONFIDENTIALITY = 0;
  static ::java::lang::String * HMAC_SHA1;
  static ::java::lang::String * HMAC_MD5;
  static ::java::lang::String * HMAC_RIPEMD_160;
  static JArray< ::java::lang::String * > * INTEGRITY_ALGORITHMS;
  static ::java::lang::String * AES;
  static ::java::lang::String * BLOWFISH;
  static JArray< ::java::lang::String * > * CONFIDENTIALITY_ALGORITHMS;
  static ::java::lang::String * OPTION_MANDATORY;
  static ::java::lang::String * OPTION_SRP_DIGEST;
  static ::java::lang::String * OPTION_REPLAY_DETECTION;
  static ::java::lang::String * OPTION_INTEGRITY;
  static ::java::lang::String * OPTION_CONFIDENTIALITY;
  static ::java::lang::String * OPTION_MAX_BUFFER_SIZE;
  static ::java::lang::String * MANDATORY_NONE;
  static ::java::lang::String * DEFAULT_MANDATORY;
  static ::java::lang::String * MD_NAME_FIELD;
  static ::java::lang::String * USER_VERIFIER_FIELD;
  static ::java::lang::String * SALT_FIELD;
  static ::java::lang::String * CONFIG_NDX_FIELD;
  static const jint MINIMUM_MODULUS_BITLENGTH = 512;
  static ::java::lang::Class class$;
} __attribute__ ((java_interface));

#endif // __gnu_javax_crypto_sasl_srp_SRPRegistry__
