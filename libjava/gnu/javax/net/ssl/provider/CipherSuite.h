
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_javax_net_ssl_provider_CipherSuite__
#define __gnu_javax_net_ssl_provider_CipherSuite__

#pragma interface

#include <java/lang/Object.h>
#include <gcj/array.h>

extern "Java"
{
  namespace gnu
  {
    namespace javax
    {
      namespace net
      {
        namespace ssl
        {
          namespace provider
          {
              class CipherAlgorithm;
              class CipherSuite;
              class KeyExchangeAlgorithm;
              class MacAlgorithm;
              class ProtocolVersion;
              class SignatureAlgorithm;
          }
        }
      }
    }
  }
  namespace java
  {
    namespace nio
    {
        class ByteBuffer;
    }
  }
  namespace javax
  {
    namespace crypto
    {
        class Cipher;
        class Mac;
    }
  }
}

class gnu::javax::net::ssl::provider::CipherSuite : public ::java::lang::Object
{

  CipherSuite(::gnu::javax::net::ssl::provider::CipherAlgorithm *, ::gnu::javax::net::ssl::provider::KeyExchangeAlgorithm *, ::gnu::javax::net::ssl::provider::SignatureAlgorithm *, ::gnu::javax::net::ssl::provider::MacAlgorithm *, jint, jint, jint, ::java::lang::String *, jboolean);
  CipherSuite(::gnu::javax::net::ssl::provider::CipherAlgorithm *, ::gnu::javax::net::ssl::provider::KeyExchangeAlgorithm *, jboolean, ::gnu::javax::net::ssl::provider::SignatureAlgorithm *, ::gnu::javax::net::ssl::provider::MacAlgorithm *, jint, jint, jint, ::java::lang::String *, jboolean);
  CipherSuite(JArray< jbyte > *);
public:
  static ::gnu::javax::net::ssl::provider::CipherSuite * forName(::java::lang::String *);
  static ::gnu::javax::net::ssl::provider::CipherSuite * forValue(jshort);
  static ::java::util::List * availableSuiteNames();
  ::gnu::javax::net::ssl::provider::CipherAlgorithm * cipherAlgorithm();
  ::javax::crypto::Cipher * cipher();
  ::gnu::javax::net::ssl::provider::MacAlgorithm * macAlgorithm();
  ::javax::crypto::Mac * mac(::gnu::javax::net::ssl::provider::ProtocolVersion *);
  ::gnu::javax::net::ssl::provider::SignatureAlgorithm * signatureAlgorithm();
  ::gnu::javax::net::ssl::provider::KeyExchangeAlgorithm * keyExchangeAlgorithm();
  jboolean isEphemeralDH();
  jint length();
  void write(::java::io::OutputStream *);
  void put(::java::nio::ByteBuffer *);
  ::gnu::javax::net::ssl::provider::CipherSuite * resolve();
  jboolean isResolved();
  jint keyLength();
  jboolean isExportable();
  jboolean isStreamCipher();
  JArray< jbyte > * id();
  jboolean equals(::java::lang::Object *);
  jint hashCode();
  ::java::lang::String * toString(::java::lang::String *);
  ::java::lang::String * toString();
  jboolean isCBCMode();
private:
  static ::java::util::List * tlsSuiteNames;
  static ::java::util::HashMap * namesToSuites;
public:
  static ::gnu::javax::net::ssl::provider::CipherSuite * TLS_NULL_WITH_NULL_NULL;
  static ::gnu::javax::net::ssl::provider::CipherSuite * TLS_RSA_WITH_NULL_MD5;
  static ::gnu::javax::net::ssl::provider::CipherSuite * TLS_RSA_WITH_NULL_SHA;
  static ::gnu::javax::net::ssl::provider::CipherSuite * TLS_RSA_EXPORT_WITH_RC4_40_MD5;
  static ::gnu::javax::net::ssl::provider::CipherSuite * TLS_RSA_WITH_RC4_128_MD5;
  static ::gnu::javax::net::ssl::provider::CipherSuite * TLS_RSA_WITH_RC4_128_SHA;
  static ::gnu::javax::net::ssl::provider::CipherSuite * TLS_RSA_EXPORT_WITH_DES40_CBC_SHA;
  static ::gnu::javax::net::ssl::provider::CipherSuite * TLS_RSA_WITH_DES_CBC_SHA;
  static ::gnu::javax::net::ssl::provider::CipherSuite * TLS_RSA_WITH_3DES_EDE_CBC_SHA;
  static ::gnu::javax::net::ssl::provider::CipherSuite * TLS_DH_DSS_EXPORT_WITH_DES40_CBC_SHA;
  static ::gnu::javax::net::ssl::provider::CipherSuite * TLS_DH_DSS_WITH_DES_CBC_SHA;
  static ::gnu::javax::net::ssl::provider::CipherSuite * TLS_DH_DSS_WITH_3DES_EDE_CBC_SHA;
  static ::gnu::javax::net::ssl::provider::CipherSuite * TLS_DH_RSA_EXPORT_WITH_DES40_CBC_SHA;
  static ::gnu::javax::net::ssl::provider::CipherSuite * TLS_DH_RSA_WITH_DES_CBC_SHA;
  static ::gnu::javax::net::ssl::provider::CipherSuite * TLS_DH_RSA_WITH_3DES_EDE_CBC_SHA;
  static ::gnu::javax::net::ssl::provider::CipherSuite * TLS_DHE_DSS_EXPORT_WITH_DES40_CBC_SHA;
  static ::gnu::javax::net::ssl::provider::CipherSuite * TLS_DHE_DSS_WITH_DES_CBC_SHA;
  static ::gnu::javax::net::ssl::provider::CipherSuite * TLS_DHE_DSS_WITH_3DES_EDE_CBC_SHA;
  static ::gnu::javax::net::ssl::provider::CipherSuite * TLS_DHE_RSA_EXPORT_WITH_DES40_CBC_SHA;
  static ::gnu::javax::net::ssl::provider::CipherSuite * TLS_DHE_RSA_WITH_DES_CBC_SHA;
  static ::gnu::javax::net::ssl::provider::CipherSuite * TLS_DHE_RSA_WITH_3DES_EDE_CBC_SHA;
  static ::gnu::javax::net::ssl::provider::CipherSuite * TLS_RSA_WITH_AES_128_CBC_SHA;
  static ::gnu::javax::net::ssl::provider::CipherSuite * TLS_DH_DSS_WITH_AES_128_CBC_SHA;
  static ::gnu::javax::net::ssl::provider::CipherSuite * TLS_DH_RSA_WITH_AES_128_CBC_SHA;
  static ::gnu::javax::net::ssl::provider::CipherSuite * TLS_DHE_DSS_WITH_AES_128_CBC_SHA;
  static ::gnu::javax::net::ssl::provider::CipherSuite * TLS_DHE_RSA_WITH_AES_128_CBC_SHA;
  static ::gnu::javax::net::ssl::provider::CipherSuite * TLS_RSA_WITH_AES_256_CBC_SHA;
  static ::gnu::javax::net::ssl::provider::CipherSuite * TLS_DH_DSS_WITH_AES_256_CBC_SHA;
  static ::gnu::javax::net::ssl::provider::CipherSuite * TLS_DH_RSA_WITH_AES_256_CBC_SHA;
  static ::gnu::javax::net::ssl::provider::CipherSuite * TLS_DHE_DSS_WITH_AES_256_CBC_SHA;
  static ::gnu::javax::net::ssl::provider::CipherSuite * TLS_DHE_RSA_WITH_AES_256_CBC_SHA;
  static ::gnu::javax::net::ssl::provider::CipherSuite * TLS_PSK_WITH_RC4_128_SHA;
  static ::gnu::javax::net::ssl::provider::CipherSuite * TLS_PSK_WITH_3DES_EDE_CBC_SHA;
  static ::gnu::javax::net::ssl::provider::CipherSuite * TLS_PSK_WITH_AES_128_CBC_SHA;
  static ::gnu::javax::net::ssl::provider::CipherSuite * TLS_PSK_WITH_AES_256_CBC_SHA;
  static ::gnu::javax::net::ssl::provider::CipherSuite * TLS_DHE_PSK_WITH_RC4_128_SHA;
  static ::gnu::javax::net::ssl::provider::CipherSuite * TLS_DHE_PSK_WITH_3DES_EDE_CBC_SHA;
  static ::gnu::javax::net::ssl::provider::CipherSuite * TLS_DHE_PSK_WITH_AES_128_CBC_SHA;
  static ::gnu::javax::net::ssl::provider::CipherSuite * TLS_DHE_PSK_WITH_AES_256_CBC_SHA;
  static ::gnu::javax::net::ssl::provider::CipherSuite * TLS_RSA_PSK_WITH_RC4_128_SHA;
  static ::gnu::javax::net::ssl::provider::CipherSuite * TLS_RSA_PSK_WITH_3DES_EDE_CBC_SHA;
  static ::gnu::javax::net::ssl::provider::CipherSuite * TLS_RSA_PSK_WITH_AES_128_CBC_SHA;
  static ::gnu::javax::net::ssl::provider::CipherSuite * TLS_RSA_PSK_WITH_AES_256_CBC_SHA;
private:
  ::gnu::javax::net::ssl::provider::CipherAlgorithm * __attribute__((aligned(__alignof__( ::java::lang::Object)))) cipherAlgorithm__;
  ::gnu::javax::net::ssl::provider::KeyExchangeAlgorithm * keyExchangeAlgorithm__;
  ::gnu::javax::net::ssl::provider::SignatureAlgorithm * signatureAlgorithm__;
  ::gnu::javax::net::ssl::provider::MacAlgorithm * macAlgorithm__;
  jboolean ephemeralDH;
  jboolean exportable;
  jboolean isStream;
  jboolean isCBCMode__;
  jint keyLength__;
  JArray< jbyte > * id__;
  ::java::lang::String * name;
  jboolean isResolved__;
public:
  static ::java::lang::Class class$;
};

#endif // __gnu_javax_net_ssl_provider_CipherSuite__
