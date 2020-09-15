# Tips-for-coding-C-learned-in-ncat
Ncat에서 배운 소소한 C언어 코딩 팁 ( Tips for coding C languages learned in Ncat )


### Ncat 코드 읽기

&nbsp;
&nbsp;

1. **옵션값을 받을 때 함수 사용**
 - getopt_long 을 이용해서 옵션을 할당

```c
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

/*
 * @예제 출처 : https://www.joinc.co.kr/w/man/3/getopt_long
 */
void help(char *argv)
{
    printf("Usage : %s options\n", argv);
    printf("--read\n--write\n--file [file name]\n--help\n");
}

int verbose_flag;

int main (int argc, char *argv[])
{
  int c;

  verbose_flag = 0;

  while (1) {
      static struct option long_options[] =
        {
       // { 인자값, 인자 여부, 리턴값, 옵션 }
          {"read", no_argument,        0, 'r'},
          {"verbose", no_argument,       &verbose_flag, 1},
          {"write", no_argument,       0, 'w'},
          {"help",   no_argument,       0, 'h'},
          {"file",  required_argument,  0, 'f'},
          {0, 0, 0, 0}
        };

      int option_index = 0;

      c = getopt_long(argc, argv, "rwhf:", long_options, &option_index);
       // getopt_long(인자개수, 인자값, short인자, long인자, 옵션 인덱스)
       // c에는 short인자가 들어가게 됨

      if (c == -1)
        break;

      switch (c) {
          case 0:
            break;

          case 'r':
            printf("read mode\n");
            break;

          case 'w':
            printf("write mode\n");
            break;

          case 'f':
            printf ("file name is `%s'\n", optarg);
            break;

          case 'h':
            help(argv[0]);
            break;

          case '?':
            help(argv[0]);
            break;
          default:
             help(argv[0]);
       }
    }

    if(verbose_flag)
      printf("verbos flag is set\n");
    return 1;
}
```

 **실행결과**

```bash
[root@localhost ncat_examples]# ./getopt_long --read --write --file test.txt
read mode
write mode
file name is `test.txt'
```
&nbsp;
&nbsp;
2.  **strspn / strchr 함수 작동 예제**

2-1. **strchr 함수 예제** 

```c

#include <stdio.h>
#include <string.h>

int main(void)
{
    char str[] = "hjun-park Github";
    char* ptr = strchr(str, 'h');

    // 한 개만 찾을 경우
    if(ptr != NULL) {
        printf("찾는 문자 : %c\n", *ptr);
        printf("찾은 후 부터의 문자열 : %s\n", ptr);
    }

    printf("\n\n");

    // 여러 개를 찾을 경우
    while(ptr != NULL) {
        printf("찾는 문자 : %c, 찾은 문자열 : %s\n", *ptr, ptr);
        ptr = strchr(ptr + 1, 'h');    //ptr + 1 이 중요합니다.
    }

    return 0;
}
```

**실행 결과**

```bash
[root@localhost ncat_examples]# ./strchr
찾는 문자 : h
찾은 후 부터의 문자열 : hjun-park Github

찾는 문자 : h, 찾은 문자열 : hjun-park Github
찾는 문자 : h, 찾은 문자열 : hub
```

2-2. **strspn 함수 예제**

```c
#include <stdio.h>
#include <string.h>

int main() {
  int i;
  char temp_text[] = "129th";
  char number_list[] = "1234567890";

  // number_list가 가지고 있는 것 중에
  // temp_text와 일치하는 값을 찾아서 그 갯수를 반환
  i = strspn(temp_text, number_list);

  printf("일치하는 개수 : %d.\n", i);

  return 0;
}
```

**실행 결과**

```bash
[root@localhost ncat_examples]# ./strspn
일치하는 개수 : 3.
```


&nbsp;
&nbsp;
3.  **모든 함수가 static 함수**

```c
## static 함수를 사용하는 이유 

1. 캡슐화를 통한 데이터 은닉
2. 이식성 장점
3. 스코프 한정자 역할 :: 코드 상에서 스코프 범위 제한함으로써 외부에 인터페이스 노출이 
되지 않는다. 다른 소스 파일에서 같은 이름으로 함수를 만들었을 때에도 제대로 동작한다. 
( static 자체가 그 파일 내에서만 사용하는 것으로 취급 )

 ex) a.c에 static void ab() 
      b.c에 static int ab()가 있을 때 static으로 만들었기 때문에 이름이 같다는 
			오류는 발생하지 않는다.
```



&nbsp;
&nbsp;
4. **예외처리를 매크로를 이용하여 처리**

```c
#include <cstdio>
#include <iostream>

// 개선 X 부분 -> 에러가 출력됨
//#define exch(x,y)    \
//        { int tmp;   \
//          tmp = x;  \
//          x = y;     \
//          y = tmp; }
//

// 개선된 부분  컴파일이 잘 되는 것을 확인
//#define exch(x,y)    \
//        do{ int tmp;   \
//          tmp = x;  \
//          x = y;     \
//          y = tmp; }while(0)

// 매크로 대신 사용하는 인라인 + 템플릿
template <typename T>
inline void exch(T x, T y)
{
    T tmp;
    x = y;
    y = tmp;

}

void do_something()
{
    printf("do_something() called\n");
}

int main()
{
    int a = 1;
    int b = 2;

    if(a<b)
        exch(a, b);
    else
        do_something();

    // call by value기 때문에 사실상 바뀌지는 않지만
    // 함수 호출에 의의를 둠
    printf("x : %d\ny : %d\n", a, b);

    return 0;
}
```

```c
## 실제 Ncat에서 사용한 예제
/* Make the port number a string to give to getaddrinfo. */
rc = Snprintf(portbuf, sizeof(portbuf), "%hu", port);

ncat_assert(rc >= 0 && (size_t) rc < sizeof(portbuf));

	
# 다음과 같이 assert문을 이용해서 예외처리 가능
#define ncat_assert(expr) \
do { \
if (!(expr)) \
bye("assertion failed: %s", #expr); \
} while (0)

1. 빈 문장(“;”)은 컴파일러에서 Warning 을 발생시키므로 이를 방지!
2. 지역변수를 할당할 수 있는 Basic block 을 쓸 수 있다.
3. 조건문에서 복잡한 문장을 사용할 수 있다.

매크로 대신에 인라인을 사용하는 것은 ? 더욱좋다.
하지만 코드가 길어지고 자료형에 종속될 수 있다.
자료형에 구속을 받는다면 인라인 템플릿을 이용해본다.
```

&nbsp;
&nbsp;

5. **return 값에 함수의 결과를 반환한다. 코드가 더 간결해진다는 장점이 있다.**

```c
# return 하면서 함수를 수행 (수행하면서 결과값 return) 간결함 UP

int resolve_multi(const char *hostname, unsigned short port,
struct sockaddr_list *sl, int af)
{
	int flags;

	flags = 0;
	if (o.nodns)
	flags |= AI_NUMERICHOST;

	return resolve_internal(hostname, port, sl, af, flags, 1);
}
```

&nbsp;
&nbsp;

6. **dotelnet**에는 telnet 협상 기능 코드가 있다. 숫자에 따라 다르다.

```c
void dotelnet(int s, unsigned char *buf, size_t bufsiz) 
 {                                                       
     unsigned char *end = buf + bufsiz, *p;              
     unsigned char tbuf[3];                              
                                                         
     for (p = buf; buf < end; p++) {                     
         if (*p != 255) /* IAC */                        
             break;                                      
                                                         
         tbuf[0] = *p++;                                 
                                                         
         /* Answer DONT for WILL or WONT */              
         if (*p == 251 || *p == 252)                     
             tbuf[1] = 254;                              
                                                         
         /* Answer WONT for DO or DONT */                
         else if (*p == 253 || *p == 254)                
             tbuf[1] = 252;                              
                                                         
         tbuf[2] = *++p;                                 
                                                         
         send(s, (const char *) tbuf, 3, 0);             
     }                                                   
 }
```

&nbsp;
&nbsp;

7. 초기화 하는 init 파트를 만들었으면 자연스레 이를 해제하는 free 파트도 함께 만들어 준다.

```c
# 자원 할당
void http_request_init(struct http_request *request) 
{                                                    
    request->method = NULL;                          
    uri_init(&request->uri);                         
    request->version = HTTP_UNKNOWN;                 
    request->header = NULL;                          
    request->content_length_set = 0;                 
    request->content_length = 0;                     
    request->bytes_transferred = 0;                  
}                                                    
               
# 자원 해제                                      
void http_request_free(struct http_request *request) 
{                                                    
    free(request->method);                           
    uri_free(&request->uri);                         
    http_header_free(request->header);               
}
```


&nbsp;
&nbsp;
8. 실패로 인해 함수를 종료할 때 **goto문을 사용**

```c
if (code != 200) {
        loguser("Proxy returned status code %d.\n", code);
        goto bail;
    }

    free(header);
    header = NULL;

    remainder = socket_buffer_remainder(&sockbuf, &len);
    Write(STDOUT_FILENO, remainder, len);

    return sd;

bail:
    if (sd != -1)
        close(sd);
    if (request != NULL)
        free(request);
    if (status_line != NULL)
        free(status_line);
    if (header != NULL)
        free(header);

    return -1;
}
```

**goto문은 return으로 대체하는 것이 좋지만 오류처리를 위해 분기하는 경우는 현업에도 사용**

 - 다음과 같이 return을 이용해서 goto를 대체할 수 있다.

```c
#include <stdio.h>

int main()
{
    int i, j;

    for ( i = 0; i < 10; i++ )
    {
        printf( "Outer loop executing. i = %d\n", i );
        for ( j = 0; j < 3; j++ )
        {
            printf( " Inner loop executing. j = %d\n", j );
            if ( i == 5 ) {
                printf( "Jumped to stop. i = %d\n", i );
                return 0;
            }
        }
    }

    /* This message does not print: */
    printf( "Loop exited. i = %d\n", i );

}
```


&nbsp;
&nbsp;
9.  여러 개의 user-defined 변수는 매크로를 쓰는 방법보다 enum을 쓰는 방법도 존재
      

```c
enum http_auth_scheme { AUTH_UNKNOWN, AUTH_BASIC, AUTH_DIGEST };                
enum http_digest_algorithm { ALGORITHM_MD5, ALGORITHM_UNKNOWN };                
enum http_digest_qop { QOP_NONE = 0, QOP_AUTH = 1 << 0, QOP_AUTH_INT = 1 << 1 };
                       
/* 아래와 같이 구조체에 enum을 사용하는 방법 */                                                         
struct http_challenge {                                                         
    enum http_auth_scheme scheme;                                               
    char *realm;                                                                
    struct {                                                                    
        char *nonce;                                                            
        char *opaque;                                                           
        enum http_digest_algorithm algorithm;                                   
        /* A bit mask of supported qop values ("auth", "auth-int", etc.). */    
        unsigned char qop;                                                      
    } digest;                                                                   
};
```

열거형은 특정한 상태 집합을 나타내야 할 때 코드 문서화 및 가독성 목적으로 매우 유용하다.

  예를 들어 함수는 함수 내부에 문제가 발생했을 때 오류 코드를 나타내기 위해 호출자에게 정수를 반환하는 경우가 많다. 일반적으로 오류 코드를 나타내는 데는 음수가 사용된다.

 ⇒ 15번하고 겹침


&nbsp;
&nbsp;

10. switch~case 문에서 enum을 사용한 Ncat 코드

```c
switch (srcaddr.storage.ss_family) {                        
     case AF_UNSPEC:                                           
       break;                                                  
     case AF_INET:                                             
       nsock_iod_set_localaddr(nsi, &srcaddr.storage,          
           sizeof(srcaddr.in));                                
       break;                                                  
#ifdef AF_INET6                                                
     case AF_INET6:                                            
       nsock_iod_set_localaddr(nsi, &srcaddr.storage,          
           sizeof(srcaddr.in6));                               
       break;                                                  
#endif                                                         
#if HAVE_SYS_UN_H                                              
     case AF_UNIX:                                             
       nsock_iod_set_localaddr(nsi, &srcaddr.storage,          
           SUN_LEN((struct sockaddr_un *)&srcaddr.storage));   
       break;                                                  
#endif                                                         
     default:                                                  
       nsock_iod_set_localaddr(nsi, &srcaddr.storage,          
           sizeof(srcaddr.storage));                           
       break;                                                  
   }
```

enum에도 확실한 영역 구분을 해주기 위해 enum class 개념 도입.

```c
#include <cstdio>
#include <iostream>

using namespace std;

int main()
{
//    enum MyFile myfile;    // 열거형 변수 선언
//    enum MyDir mydir;
    enum class MyFile {
        ReadFile = -1,
        WriteFile
    };

    enum class MyDir {
        OpenDir,
        CloseDir
    };

    MyFile myfile = MyFile::ReadFile;    // 열거형 값 할당A
    MyDir mydir = MyDir::OpenDir;

//    if(myfile == mydir) {   // 비교불가. 컴파일 에러,
//                            // 같은 enum끼리만 비교해야함
        switch (myfile)
        {
            case MyFile::ReadFile:
                printf("ReadFile %d\n", MyFile::ReadFile);
                break;
            case MyFile::WriteFile:
                printf("WriteFile\n");
                break;
            default:
                break;
        }
//    }
//
//    else {
//        cout << "myfile and mydir are different things" << endl;
//    }

    return 0;
}
```


&nbsp;
&nbsp;
11. 옵션을 구조체로 선언하고 option_init 함수를 만들어서 초기화를 진행

```c
/* Initializes global options to their default values. */
void options_init(void)                                  
{                                                        
    o.verbose = 0;                                       
    o.debug = 0;                                         
    o.target = NULL;                                     
    o.af = AF_UNSPEC;                                    
    o.proto = IPPROTO_TCP;                               
    o.broker = 0;                                        
    o.listen = 0;                                        
    o.keepopen = 0;                                      
    o.sendonly = 0;                                      
    o.recvonly = 0;                                      
    o.noshutdown = 0;                                    
    o.telnet = 0;                                        
    o.linedelay = 0;                                     
    o.chat = 0;                                          
    o.nodns = 0;
	...

// 옵션은 getopt_long을 사용하기 위해 구조체 형식으로 생성 ( 1번 참고 )
struct option long_options[] = {
        {"4",               no_argument,        NULL,         '4'},
        {"6",               no_argument,        NULL,         '6'},
#if HAVE_SYS_UN_H
        {"unixsock",        no_argument,        NULL,         'U'},
#endif
#if HAVE_LINUX_VM_SOCKETS_H
        {"vsock",           no_argument,        NULL,         0},
#endif
        {"crlf",            no_argument,        NULL,         'C'},
        {"g",               required_argument,  NULL,         'g'},
        {"G",               required_argument,  NULL,         'G'},
        {"exec",            required_argument,  NULL,         'e'},
        {"sh-exec",         required_argument,  NULL,         'c'},
#ifdef HAVE_LUA
        {"lua-exec",        required_argument,  NULL,         0},
        {"lua-exec-internal",required_argument, NULL,         0},
#endif
        {"max-conns",       required_argument,  NULL,         'm'},
        {"help",            no_argument,        NULL,         'h'},
        {"delay",           required_argument,  NULL,         'd'},
        {"listen",          no_argument,        NULL,         'l'},
        {"output",          required_argument,  NULL,         'o'},
        {"hex-dump",        required_argument,  NULL,         'x'},
        {"append-output",   no_argument,        NULL,         0},
        {"idle-timeout",    required_argument,  NULL,         'i'},
        {"keep-open",       no_argument,        NULL,         'k'},
        {"recv-only",       no_argument,        &o.recvonly,  1},
        {"source-port",     required_argument,  NULL,         'p'},
        {"source",          required_argument,  NULL,         's'},
```


&nbsp;
&nbsp;
12. pcap_util에서 보면 기존 함수를 가지고 safe 한  함수로 만들어 사용하고 있음

```c
ssize_t Read(int fd, void *buf, size_t count)
{
    ssize_t ret;

    ret = read(fd, buf, count);
    if (ret < 0)
        die("read");

    return ret;
}

int Setsockopt(int s, int level, int optname, const void *optval,
                    socklen_t optlen)
{
    int ret;

    ret = setsockopt(s, level, optname, (const char *) optval, optlen);
    if (ret < 0)
        die("setsockopt");

    return ret;
}
```
