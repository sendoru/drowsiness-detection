# Drowsiness driving detection system

## Description
컴퓨터 비전 및 머신 러닝을 이용해 운전자의 졸음 상태를 판별하고, 특정 수준 이상에서 경고를 울리게 하는 프로그램입니다. 안면 및 안구를 검출하여 안구 크기에 따라 운전자가 눈을 뜨고 있는지 감고 있는지 판단하고, 눈을 감은 시간에 따라 운전자의 졸음 상태를 판별합니다.

https://github.com/woorimlee/drowsiness-detection 의 코드를 Raspberry Pi에서 작동되도록 수정한 버전입니다. 구체적인 수정 사항은 다음과 같습니다.

* 모니터가 없는 상황에서도 작동할 수 있도록 수정
* Raspberry Pi에서 검출한 결과를 Arduino로 전송하기 위해 GPIO output 추가
* 일부 모듈 의존성 제거

## Setup
Python3 기반입니다.
1. Raspberry Pi에 설치한 카메라를 활성화합니다.

    1-1) 터미널에 ```sudo raspi-config```를 입력합니다. <br>
    1-2) ```Interfacing Options```을 선택합니다.<br>
    1-3) ```Camera```를 선택하고, 활성화시키겠냐고 물어보는 창이 뜨면 ```Yes```를 선택합니다.<br>
    1-4) Raspberry Pi를 재부팅합니다.<br>

2. 이 폴더의 파일을 Raspberry Pi 저장소 내에 저장합니다.

3. 다음 패키지들을 설치합니다.
    * numpy
    * imutils
    * dlib
    * cv2
    * scipy


    openCV는 pip3에서 한 번에 설치가 되지 않습니다. 다음 링크를 참조하여 진행하면 도움이 됩니다. <br>
    https://fishpoint.tistory.com/6082

    여기서 ```drowsiness_detector.py```를 실행해 작동이 제대로 되는지 한 번 확인하는 것을 권장합니다.

4. ```drowsiness_detector.py```가 시작 프로그램으로 실행되도록 등록합니다. 라즈베리 파이가 외부 모니터에 연결되지 않고 GUI 환경이 로딩되지 않았을 때도 작동하게 하기 위해서는, chontab을 이용하면 됩니다. https://www.dexterindustries.com/howto/auto-run-python-programs-on-the-raspberry-pi/ 를 참조하여 진행해 주세요.