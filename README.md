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

------

### project 3

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
