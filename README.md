# 2021_OS
'2021-2 semester &lt;Operating System> assignment + project' purpose repository
------

### project 1
1. 리눅스 커널 설치
```
  wget https:// www.kernel.org/pub/linux/kernel/v5.x/linux 5.13.12.tar.xz
```

2. root 권한에서 커널 컴파일을 위한 라이브러리 설치
```
  apt get install vim make gcc kernel package libncurses5 dev bison flex libssl dev
```

3. 새로 설치할 커널의 설정 파일 변경을 위해 menuconfig 생성
```
  make menuconfig
```

4. 커널 컴파일을 진행해 이미지 파일 생성
```
  make kpkg initrd revision=1.0 kernel_image
```

5. 생성된 deb 파일을 커널 이미지로 부팅하는 작업 진행
```
  dpkg I linux image 5.13.12_1.0_amd64.deb
```
------

### project 2
- 리눅스 명령어 ps, top, lscpu와 같은 역할을 수행하는 명령어 myps, mytop, mylscpu 구현
------

### project 3
- 시스템 콜 함수 추가 구현
1. ```/usr/src/linux/linux-5.11.22/arch/x86/entry/syscalls``` 디렉토리로 이동 후 ```syscall_xx.tbl``` 파일 편집
2. 시스템 콜 테이블 등록
3. ```/usr/src/linux/linux-5.11.22/include/linux``` 디렉토리로 이동 후 ```syscalls.h```을 vi 편집기로 열기
4. ```asmlinkage```를 앞에 붙여 어셈블리 코드에서도 C 함수 호출이 가능하도록 시스템콜 헤더 파일에 등록
5. ```/usr/src/linux/linux-5.11.22/kernel``` 디렉토리로 이동 후 추가할 시스템 콜의 구현 파일 편집
6. Makefile 편집
7. 커널 소스 디렉토리로 이동해 새로 컴파일 후 재부팅, revision 사이 충돌 발생을 막기 위해 다른 값 입력해 컴파일 실시
8. ```dmesg``` 명령어 통해 커널 로그 출력해 추가된 시스템콜 함수 정상 실행 확인
------

### project 4
- 리눅스 커널의 스케줄링 정책을 (기본 CFS, 조정된 Nice 값을 적용한 CFS) 확인할 수 있는 프로그램 작성

- 구현에 사용하는 리눅스 커널의 버전은 5.11.22로 설정
- 리눅스 커널의 스케줄링 정책은 기본적으로 CFS(Completly Fair Scheduler)를 사용함
- fork()를 통해 총 21개의 자식 프로세스를 생성하고, 생성되는 프로세스들에게 (기본 CFS, 조정된 Nice 값을 적용한 CFS) 스케줄링 정책을 적용하는 프로그램 구현
- Nice 값을 조정하지 않고 적용되는 기본 CFS와 생성되는 프로세스들을 3개의 그룹(높은 Nice 값, 기본 Nice 값, 낮은 Nice 값)으로 나누어 적용하는 CFS

  - [스케줄링 관련 시스템 호출 및 고정 우선순위 지정](https://jeongchul.tistory.com/95)
  - [프로세스의 nice 값을 조정해 우선순위 차등 부여](https://wiseworld.tistory.com/64)
  - [nice 명령어 사용](https://jhnyang.tistory.com/394)
  - [스케줄링 정책 관련 문서](http://www.iamroot.org/xe/index.php?mid=Programming&document_srl=14564)
  
------

### project 5
- 가상메모리 관리 기법의 하나인 <페이지 교체 기법> 중 OPT, FIFO, LRU, Second-Chance를 구현하고 동작 과정을 보여주는 시뮬레이터 구현
  - [페이지 교체 기법 알고리즘](https://m.blog.naver.com/xowns4817/221226671491)
