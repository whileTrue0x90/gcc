
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __javax_transaction_TransactionManager__
#define __javax_transaction_TransactionManager__

#pragma interface

#include <java/lang/Object.h>
extern "Java"
{
  namespace javax
  {
    namespace transaction
    {
        class Transaction;
        class TransactionManager;
    }
  }
}

class javax::transaction::TransactionManager : public ::java::lang::Object
{

public:
  virtual void begin() = 0;
  virtual void commit() = 0;
  virtual jint getStatus() = 0;
  virtual ::javax::transaction::Transaction * getTransaction() = 0;
  virtual void resume(::javax::transaction::Transaction *) = 0;
  virtual void rollback() = 0;
  virtual void setRollbackOnly() = 0;
  virtual void setTransactionTimeout(jint) = 0;
  virtual ::javax::transaction::Transaction * suspend() = 0;
  static ::java::lang::Class class$;
} __attribute__ ((java_interface));

#endif // __javax_transaction_TransactionManager__
