
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __java_security_cert_X509CertSelector__
#define __java_security_cert_X509CertSelector__

#pragma interface

#include <java/lang/Object.h>
#include <gcj/array.h>

extern "Java"
{
  namespace gnu
  {
    namespace java
    {
      namespace security
      {
          class OID;
      }
    }
  }
  namespace java
  {
    namespace math
    {
        class BigInteger;
    }
    namespace security
    {
        class PublicKey;
      namespace cert
      {
          class Certificate;
          class X509CertSelector;
          class X509Certificate;
      }
      namespace spec
      {
          class X509EncodedKeySpec;
      }
    }
  }
  namespace javax
  {
    namespace security
    {
      namespace auth
      {
        namespace x500
        {
            class X500Principal;
        }
      }
    }
  }
}

class java::security::cert::X509CertSelector : public ::java::lang::Object
{

public:
  X509CertSelector();
  virtual ::java::security::cert::X509Certificate * getCertificate();
  virtual void setCertificate(::java::security::cert::X509Certificate *);
  virtual ::java::math::BigInteger * getSerialNumber();
  virtual void setSerialNumber(::java::math::BigInteger *);
  virtual ::java::lang::String * getIssuerAsString();
  virtual JArray< jbyte > * getIssuerAsBytes();
  virtual void setIssuer(::java::lang::String *);
  virtual void setIssuer(JArray< jbyte > *);
  virtual ::java::lang::String * getSubjectAsString();
  virtual JArray< jbyte > * getSubjectAsBytes();
  virtual void setSubject(::java::lang::String *);
  virtual void setSubject(JArray< jbyte > *);
  virtual JArray< jbyte > * getSubjectKeyIdentifier();
  virtual void setSubjectKeyIdentifier(JArray< jbyte > *);
  virtual JArray< jbyte > * getAuthorityKeyIdentifier();
  virtual void setAuthorityKeyIdentifier(JArray< jbyte > *);
  virtual ::java::util::Date * getCertificateValid();
  virtual void setCertificateValid(::java::util::Date *);
  virtual ::java::util::Date * getPrivateKeyValid();
  virtual void setPrivateKeyValid(::java::util::Date *);
  virtual ::java::lang::String * getSubjectPublicKeyAlgID();
  virtual void setSubjectPublicKeyAlgID(::java::lang::String *);
  virtual ::java::security::PublicKey * getSubjectPublicKey();
  virtual void setSubjectPublicKey(::java::security::PublicKey *);
  virtual void setSubjectPublicKey(JArray< jbyte > *);
  virtual JArray< jboolean > * getKeyUsage();
  virtual void setKeyUsage(JArray< jboolean > *);
  virtual ::java::util::Set * getExtendedKeyUsage();
  virtual void setExtendedKeyUsage(::java::util::Set *);
  virtual jboolean getMatchAllSubjectAltNames();
  virtual void setMatchAllSubjectAltNames(jboolean);
  virtual void setSubjectAlternativeNames(::java::util::Collection *);
  virtual void addSubjectAlternativeName(jint, ::java::lang::String *);
  virtual void addSubjectAlternativeName(jint, JArray< jbyte > *);
  virtual JArray< jbyte > * getNameConstraints();
  virtual void setNameConstraints(JArray< jbyte > *);
  virtual jint getBasicConstraints();
  virtual void setBasicConstraints(jint);
  virtual jboolean match(::java::security::cert::Certificate *);
  virtual ::java::lang::String * toString();
  virtual ::java::lang::Object * clone();
private:
  static jboolean checkOid(JArray< jint > *);
  static ::java::lang::String * AUTH_KEY_ID;
  static ::java::lang::String * SUBJECT_KEY_ID;
  static ::java::lang::String * NAME_CONSTRAINTS_ID;
  jint __attribute__((aligned(__alignof__( ::java::lang::Object)))) basicConstraints;
  ::java::security::cert::X509Certificate * cert;
  ::java::math::BigInteger * serialNo;
  ::javax::security::auth::x500::X500Principal * issuer;
  ::javax::security::auth::x500::X500Principal * subject;
  JArray< jbyte > * subjectKeyId;
  JArray< jbyte > * authKeyId;
  JArray< jboolean > * keyUsage;
  ::java::util::Date * certValid;
  ::gnu::java::security::OID * sigId;
  ::java::security::PublicKey * subjectKey;
  ::java::security::spec::X509EncodedKeySpec * subjectKeySpec;
  ::java::util::Set * keyPurposeSet;
  ::java::util::List * altNames;
  jboolean matchAllNames;
  JArray< jbyte > * nameConstraints;
  ::java::util::Set * policy;
public:
  static ::java::lang::Class class$;
};

#endif // __java_security_cert_X509CertSelector__
