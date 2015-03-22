#pragma once

 ///MD5的结果数据长度
 static const size_t MD5_HASH_SIZE   = 16;
 ///SHA1的结果数据长度
 static const size_t SHA1_HASH_SIZE  = 20;
 
 namespace HashUtility
 {
 
 
 /*!
 @brief      求某个内存块的MD5，
 @return     unsigned char* 返回的的结果，
 @param[in]  buf    求MD5的内存BUFFER指针
 @param[in]  size   BUFFER长度
 @param[out] result 结果
 */
 unsigned char *md5(const unsigned char *buf,
                    size_t size,
                    unsigned char result[MD5_HASH_SIZE]);
 
 
 /*!
 @brief      求内存块BUFFER的SHA1值
 @return     unsigned char* 返回的的结果
 @param[in]  buf    求SHA1的内存BUFFER指针
 @param[in]  size   BUFFER长度
 @param[out] result 结果
 */
 unsigned char *sha1(const unsigned char *buf,
                     size_t size,
                     unsigned char result[SHA1_HASH_SIZE]);

 std::string toString( unsigned char* pResult, size_t len );

 };

