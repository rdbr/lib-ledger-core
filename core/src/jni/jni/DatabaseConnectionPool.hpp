// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from database.djinni

#ifndef DJINNI_GENERATED_DATABASECONNECTIONPOOL_HPP_JNI_
#define DJINNI_GENERATED_DATABASECONNECTIONPOOL_HPP_JNI_

#include "../../api/DatabaseConnectionPool.hpp"
#include "djinni_support.hpp"

namespace djinni_generated {

class DatabaseConnectionPool final : ::djinni::JniInterface<::ledger::core::api::DatabaseConnectionPool, DatabaseConnectionPool> {
public:
    using CppType = std::shared_ptr<::ledger::core::api::DatabaseConnectionPool>;
    using CppOptType = std::shared_ptr<::ledger::core::api::DatabaseConnectionPool>;
    using JniType = jobject;

    using Boxed = DatabaseConnectionPool;

    ~DatabaseConnectionPool();

    static CppType toCpp(JNIEnv* jniEnv, JniType j) { return ::djinni::JniClass<DatabaseConnectionPool>::get()._fromJava(jniEnv, j); }
    static ::djinni::LocalRef<JniType> fromCppOpt(JNIEnv* jniEnv, const CppOptType& c) { return {jniEnv, ::djinni::JniClass<DatabaseConnectionPool>::get()._toJava(jniEnv, c)}; }
    static ::djinni::LocalRef<JniType> fromCpp(JNIEnv* jniEnv, const CppType& c) { return fromCppOpt(jniEnv, c); }

private:
    DatabaseConnectionPool();
    friend ::djinni::JniClass<DatabaseConnectionPool>;
    friend ::djinni::JniInterface<::ledger::core::api::DatabaseConnectionPool, DatabaseConnectionPool>;

    class JavaProxy final : ::djinni::JavaProxyHandle<JavaProxy>, public ::ledger::core::api::DatabaseConnectionPool
    {
    public:
        JavaProxy(JniType j);
        ~JavaProxy();

        std::shared_ptr<::ledger::core::api::DatabaseConnection> getConnection() override;

    private:
        friend ::djinni::JniInterface<::ledger::core::api::DatabaseConnectionPool, ::djinni_generated::DatabaseConnectionPool>;
    };

    const ::djinni::GlobalRef<jclass> clazz { ::djinni::jniFindClass("co/ledger/core/DatabaseConnectionPool") };
    const jmethodID method_getConnection { ::djinni::jniGetMethodID(clazz.get(), "getConnection", "()Lco/ledger/core/DatabaseConnection;") };
};

}  // namespace djinni_generated
#endif //DJINNI_GENERATED_DATABASECONNECTIONPOOL_HPP_JNI_
