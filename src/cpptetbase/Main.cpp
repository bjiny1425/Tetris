#include <iostream>
#include <cstdlib>
#include <ctime>
#include <stdio.h>
#include <termios.h>

#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#include "colors.h"
#include "Matrix.h"

using namespace std;

/**************************************************************/
/**************** Linux System Functions **********************/
/**************************************************************/

// getch() 함수에서 사용할 함수 미리 선언
char saved_key = 0;    // 이전에 저장된 문자를 저장하는 변수
int tty_raw(int fd);   // 터미널을 Raw 모드로 변경하는 함수. 텔레타입.
int tty_reset(int fd); // 터미널을 기본 모드로 되돌리는 함수

/* Read 1 character - echo defines echo mode */
// 사용자로부터 1개의 문자(char)를 입력받아 반환하는 함수
char getch()
{
  char ch;
  int n;
  while (1)
  {                      // 무한 루프 시작
    tty_raw(0);          // 터미널을 Raw 모드로 변경
    n = read(0, &ch, 1); // 문자 1개 읽어옴
    // 0 : 'stdin' 파일 디스크립터, &ch : ch 변수의 메모리 주소, 1 : 읽어올 문자의 개수
    tty_reset(0); // 터미널을 기본 모드로 되돌림
    if (n > 0)    // 성공적이므로 루프 종료!
      break;
    else if (n < 0)
    { // 에러가 발생한 경우 ㅠㅠ
      if (errno == EINTR)
      { // errno : 에러 발생 시 에러 종류를 저장하는 변수. 인터럽트 발생
        if (saved_key != 0)
        {                 // 이전에 저장한 문자가 있음
          ch = saved_key; // ch에 값 저장
          saved_key = 0;  // 0으로 초기화
          break;
        }
      }
    }
  }
  return ch; // ch 반환
}

/*
이벤트(시그널) 발생하면 해당 시그널에 해당하는 핸들러가 실행됨
-> 프로세스가 예기치 않은 이벤트를 감지하고 처리.
sigint (인터럽트) : 예외 사항 발생
sigkill (강제 종료)
sigterm (일반적인 종료)
sigsegv (세그멘테이션 오류)
sigalrm (알람 타이머 만료)
sa : signal action
flags : 상수 값들의 집합
*/

void sigint_handler(int signo)
{ // sigint 시그널을 처리하는 핸들러 함수
  // cout << "SIGINT received!" << endl;
  // do nothing;
}

void sigalrm_handler(int signo)
{                  // sigalrm 시그널을 처리하는 핸들러 함수
  alarm(1);        // 1초 후에 시그널을 발생시킴
  saved_key = 's'; // s를 저장
}

void registerInterrupt()
{                             // sigint 시그널 핸들러를 등록하는 함수
  struct sigaction act, oact; // 시그널 핸들러 함수가 호출되는 방법 지정하는 구조체
  // 멤버 변수 : sa_handler, sa_mask, sa_flags
  act.sa_handler = sigint_handler; // SIGINT 시그널을 처리할 핸들러 지정
  sigemptyset(&act.sa_mask);       // 핸들러 실행 도중 블록될 시그널 집합 지정.
  // 빈 집합을 설정했기 때문에 다른 시그널을 블록하지 않음
#ifdef SA_INTERRUPT            // 컴파일러에 정의되어 있음?
  act.sa_flags = SA_INTERRUPT; // 블록된 시그널을 받으면 중단하고 해당 시그널 처리할 핸들러 호출
#else
  act.sa_flags = 0; // 정의 안돼있으면 0으로 설정
#endif
  if (sigaction(SIGINT, &act, &oact) < 0)
  { // sigint 시그널 핸들러 등록
    // (처리할 시그널의 번호, 시그널 처리 방법을 지정하는 구조체의 포인터, 이전에 설정되었던 시그널 처리 방법을 저장하는 구조체)
    // 성공 0. 실패 -1 -> cerr (즉시 출력. 오류 메시지 빠르게 확인 가능)
    cerr << "sigaction error" << endl;
    exit(1); // 종료
  }
}

void registerAlarm()
{                                   // sigalrm 처리하는 핸들러 함수
  struct sigaction act, oact;       // 구조체 이용. 처리 방법 설정
  act.sa_handler = sigalrm_handler; // act 구조체 변수에 저 함수를 시그널 핸들러로 등록
  sigemptyset(&act.sa_mask);        // 시그널 집합 비움
#ifdef SA_INTERRUPT
  act.sa_flags = SA_INTERRUPT; // 시그널 핸들러 실행 도중에 다른 시그널 블록
#else
  act.sa_flags = 0;
#endif
  if (sigaction(SIGALRM, &act, &oact) < 0)
  { // sigalrm 시그널 핸들러 등록
    // oact 구조체 변수 이전에 등록된 핸들러 정보가 저장됨
    cerr << "sigaction error" << endl;
    exit(1);
  }
  alarm(1); // 1초마다 sigalrm 시그널을 발생시킴.
  // 시그널이 발생하면 sigalrm_handler() 함수가 실행됨
}

/**************************************************************/
/**************** Tetris Blocks Definitions *******************/
/**************************************************************/
#define MAX_BLK_TYPES 7   // 블록 종류 수
#define MAX_BLK_DEGREES 4 // 블록 회전 최대 수

// 1차원 배열
//  0~3 : 블록의 형태, 4 : 블록이 이동하는 방향
//  -1 : 임의로 설정함. 실제로는 사용 x

/* 첫 번째 블록
      ■■
      ■■     */
int T0D0[] = {1, 1, 1, 1, -1};
int T0D1[] = {1, 1, 1, 1, -1};
int T0D2[] = {1, 1, 1, 1, -1};
int T0D3[] = {1, 1, 1, 1, -1};

/* 두 번째 블록
       ■
     ■■■    */
int T1D0[] = {0, 1, 0, 1, 1, 1, 0, 0, 0, -1};
int T1D1[] = {0, 1, 0, 0, 1, 1, 0, 1, 0, -1};
int T1D2[] = {0, 0, 0, 1, 1, 1, 0, 1, 0, -1};
int T1D3[] = {0, 1, 0, 1, 1, 0, 0, 1, 0, -1};

/* 세 번째 블록
     ■
     ■■■    */
int T2D0[] = {1, 0, 0, 1, 1, 1, 0, 0, 0, -1};
int T2D1[] = {0, 1, 1, 0, 1, 0, 0, 1, 0, -1};
int T2D2[] = {0, 0, 0, 1, 1, 1, 0, 0, 1, -1};
int T2D3[] = {0, 1, 0, 0, 1, 0, 1, 1, 0, -1};

/* 네 번째 블록
         ■
     ■■■    */
int T3D0[] = {0, 0, 1, 1, 1, 1, 0, 0, 0, -1};
int T3D1[] = {0, 1, 0, 0, 1, 0, 0, 1, 1, -1};
int T3D2[] = {0, 0, 0, 1, 1, 1, 1, 0, 0, -1};
int T3D3[] = {1, 1, 0, 0, 1, 0, 0, 1, 0, -1};

/* 다섯 번째 블록
        ■
      ■■
      ■       */
int T4D0[] = {0, 1, 0, 1, 1, 0, 1, 0, 0, -1};
int T4D1[] = {1, 1, 0, 0, 1, 1, 0, 0, 0, -1};
int T4D2[] = {0, 1, 0, 1, 1, 0, 1, 0, 0, -1};
int T4D3[] = {1, 1, 0, 0, 1, 1, 0, 0, 0, -1};

/* 여섯 번째 블록
      ■
      ■■
        ■     */
int T5D0[] = {0, 1, 0, 0, 1, 1, 0, 0, 1, -1};
int T5D1[] = {0, 0, 0, 0, 1, 1, 1, 1, 0, -1};
int T5D2[] = {0, 1, 0, 0, 1, 1, 0, 0, 1, -1};
int T5D3[] = {0, 0, 0, 0, 1, 1, 1, 1, 0, -1};

/* 일곱 번째 블록
     ■■■■    */
int T6D0[] = {0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, -1};
int T6D1[] = {0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, -1};
int T6D2[] = {0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, -1};
int T6D3[] = {0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, -1};

int *setOfBlockArrays[] = {
  T0D0, T0D1, T0D2, T0D3,
  T1D0, T1D1, T1D2, T1D3,
  T2D0, T2D1, T2D2, T2D3,
  T3D0, T3D1, T3D2, T3D3,
  T4D0, T4D1, T4D2, T4D3,
  T5D0, T5D1, T5D2, T5D3,
  T6D0, T6D1, T6D2, T6D3,
};

void drawScreen(Matrix *screen, int wall_depth) // 게임 스크린을 저장하는 2차원 배열
{
  int dy = screen->get_dy();         // 화면의 높이
  int dx = screen->get_dx();         // 화면의 너비
  int dw = wall_depth;               // 벽 두께
  int **array = screen->get_array(); // 스크린 객체 내부의 화면 정보 배열

  for (int y = 0; y < dy - dw + 1; y++) { // 행(세로)
    for (int x = dw - 1; x < dx - dw + 1; x++) { // 열(가로)
      if (array[y][x] == 0)
        cout << color_red << "□ " << color_normal;
      else if (array[y][x] == 1)
        cout << color_green << "■ " << color_normal;
      else if (array[y][x] == 10)
        cout << "◈ ";
      else if (array[y][x] == 20)
        cout << "★ ";
      else if (array[y][x] == 30)
        cout << "● ";
      else if (array[y][x] == 40)
        cout << "◆ ";
      else if (array[y][x] == 50)
        cout << "▲ ";
      else if (array[y][x] == 60)
        cout << "♣ ";
      else if (array[y][x] == 70)
        cout << "♥ ";
      else
        cout << "XX";
    }
    cout << endl;
  }
}

/**************************************************************/
/******************** Tetris Main Loop ************************/
/**************************************************************/

#define SCREEN_DY 10
#define SCREEN_DX 10
#define SCREEN_DW 3 

#define ARRAY_DY (SCREEN_DY + SCREEN_DW) // 스크린에 벽돌이 추가된 전체 높이
#define ARRAY_DX (SCREEN_DX + 2 * SCREEN_DW)

int arrayScreen[ARRAY_DY][ARRAY_DX] = {

    {1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1},
    {1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1},
    {1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1},
    {1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1},
    {1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1},
    {1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1},
    {1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1},
    {1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1},
    {1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1},
    {1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
};


void deleteFullLines(Matrix *iScreen) {
  Matrix *newTop = new Matrix(1, 10);
  static int score;
  int deletedLines = 0;
  int line = 9;

  while(line >= 0) {
    Matrix *currLine = iScreen->clip(line, 3, line+1, 13);
    if ((currLine->sum()) == 10) {
      for (int i = line-1; i >= 0; i--) {
        Matrix *temp = iScreen->clip(i, 3, i+1, 13);
        iScreen->paste(temp, i+1, 3);
        delete temp;
      }
      deletedLines++;
      iScreen->paste(newTop, 0, 3);
      delete currLine;
      Matrix *currLine = iScreen->clip(line, 3, line+1, 13);
    }
    else line--;
    delete currLine;
  }
  score += 10*deletedLines;
  std::cout << "Score: " << score << std::endl; // 현재 점수 출력
  delete newTop;
}


int main(int argc, char *argv[])
{
  char key;
  int idxBlockType;
  int idxBlockDegree = 0;
  int top = 0, left = 4;
  int collision = 0;

  /*
  Matrix A((int *) arrayBlk, 3, 3); //arrayBlk 2차원 배열 이용하여 3x3 행렬 A 생성
  // (int *) arrayBlk -> 메모리 주소 입력 받음
  Matrix B(A); // A를 이용하여 B 생성. 복사 생성자 호출하는 코드
  Matrix C(A);
  Matrix D; // 디폴트 생성자 호출. 빈 행렬
  D = A + B + C;
  cout << D << endl;
  return 0;

  srand((unsigned int)time(NULL));
  blkType = rand() % MAX_BLK_TYPES;
  cout << "blkType = " << blkType << endl;
  */

  Matrix *setOfBlockObjects[MAX_BLK_TYPES][MAX_BLK_DEGREES]; // 7가지 블록의 객체 생성
  for (int t = 0; t < MAX_BLK_TYPES; t++) {
    for (int d = 0; d < MAX_BLK_DEGREES; d++) {
      int *blockArray = setOfBlockArrays[t * MAX_BLK_DEGREES + d]; // setOfBlockArrays에서 int형 포인터 가져오기
      int size = (t == 0 ? 2 : t == 6 ? 4 : 3); // 블록의 크기 결정

      setOfBlockObjects[t][d] = new Matrix(blockArray, size, size); // Matrix 클래스의 객체 생성하여 setOfBlockObjects에 저장
    }
  }

  srand((unsigned int)time(NULL)); // 최초 1회만 호출
    
  // 첫 번째 블록 선택
  idxBlockType = rand() % MAX_BLK_TYPES;
  Matrix *currBlk = setOfBlockObjects[idxBlockType][idxBlockDegree];


  Matrix *iScreen = new Matrix((int *)arrayScreen, ARRAY_DY, ARRAY_DX); // 화면 상태를 나타내는 행렬
  // arrayScreen 내용을 포함하는 새로운 Matrix 객체를 동적으로 생성. 그 객체의 포인터 반환
  Matrix *tempBlk = iScreen->clip(top, left, top + currBlk->get_dy(), left + currBlk->get_dx()); // 이동 가능한 영역
  Matrix *tempBlk2 = tempBlk->add(currBlk);                                                      // tempBlk 행렬에 현재 블록을 더함
  delete tempBlk;                                                                                // 메모리 누수 방지
  // tempBlk = tempBlk->add(currBlk);

  Matrix *oScreen = new Matrix(iScreen); // iScreen + 현재 블록 추가
  oScreen->paste(tempBlk2, top, left);   // 현재 블록을 oScreen의 top, left 위치에 붙임
  delete tempBlk2;
  drawScreen(oScreen, SCREEN_DW);
  delete oScreen;



  // 메인 루프
  while ((key = getch()) != 'q') {
    switch (key) {
      case 'a': left--; break;
      case 'd': left++; break;
      case 's': top++; break;
      case 'w': { // 90도 회전
        idxBlockDegree = (idxBlockDegree + 1) % 4;
        currBlk = setOfBlockObjects[idxBlockType][idxBlockDegree];
        break;
        
      }
      case ' ': {
        while (true) { // 블록이 바닥에 닿을 때까지 루프
          top++;
          tempBlk = iScreen->clip(top, left, top + currBlk->get_dy(), left + currBlk->get_dx());
          tempBlk2 = tempBlk->add(currBlk);
          delete tempBlk;
          if(tempBlk2->anyGreaterThan(1)) {
            top--;
            collision=1;
            break;
          }
          delete tempBlk2;
        }
        delete tempBlk2;
        break;
      }
      default: cout << "wrong key input" << endl;
    }

    // 블록 위치 계산
    tempBlk = iScreen->clip(top, left, top + currBlk->get_dy(), left + currBlk->get_dx());
    tempBlk2 = tempBlk->add(currBlk);
    delete tempBlk;

    if (tempBlk2->anyGreaterThan(1)) { // 블록 충돌 시 이전 위치로 되돌림
      switch (key) {
        case 'a': left++; break;
        case 'd': left--; break;
        case 's': {
          top--; 
          collision=1;
          break;
        }
        case 'w': { // 90도 회전
          idxBlockDegree = (idxBlockDegree + 3) % 4;
          currBlk = setOfBlockObjects[idxBlockType][idxBlockDegree];
          break;
        }
        case ' ': { // 블록이 바닥에 닿을 때까지 루프를 돌며 위치 계산
          top--;
          collision = 1;
          oScreen = new Matrix(iScreen);
          oScreen->paste(tempBlk2, top, left);
          delete tempBlk2;
          delete oScreen;
          break;
        }
        default:
          cout << "wrong key input" << endl;
      }
      delete tempBlk2;
      tempBlk = iScreen->clip(top, left, top + currBlk->get_dy(), left + currBlk->get_dx());
      tempBlk2 = tempBlk->add(currBlk);
      delete tempBlk;
    }

    // 블록이 바닥에 닿은 후, 마지막 출력
    oScreen = new Matrix(iScreen);
    oScreen->paste(tempBlk2, top, left);
    delete tempBlk2;
    drawScreen(oScreen, SCREEN_DW);

    if (collision) {
      idxBlockType = rand() % MAX_BLK_TYPES;
      idxBlockDegree = 0;
      currBlk = setOfBlockObjects[idxBlockType][idxBlockDegree];
      top = 0;
      left = iScreen->get_dx() / 2 - currBlk->get_dx() / 2;
      delete iScreen;

      deleteFullLines(oScreen);

      iScreen = new Matrix(oScreen);
      delete oScreen;

      tempBlk = iScreen->clip(top, left, top + currBlk->get_dy(), left + currBlk->get_dx());
      tempBlk2 = tempBlk->add(currBlk);
      delete tempBlk;

      if (tempBlk2->anyGreaterThan(1))
      {
        delete tempBlk2;
        cout << "Game Over!" << endl;
        break;
      } 
      
      oScreen = new Matrix(iScreen);
      oScreen->paste(tempBlk2, top, left);
      delete tempBlk2;
      drawScreen(oScreen, SCREEN_DW); // 화면에 그려라. 실제 출력할 값의 시작 위치를 조정

      collision = 0; // collision 변수 초기화
    }
    delete oScreen;
  }

  for (int t = 0; t < MAX_BLK_TYPES; t++) {
    for (int d = 0; d < MAX_BLK_DEGREES; d++) {
      delete setOfBlockObjects[t][d];
    }
  }

  delete iScreen;
  // delete currBlk;
  // delete tempBlk;
  // delete tempBlk2;
  // delete oScreen;

  cout << "(nAlloc, nFree) = (" << Matrix::get_nAlloc() << ',' << Matrix::get_nFree() << ")" << endl;  // 값이 다르면 메모리 누수
  cout << "Program terminated!" << endl;

  return 0;
}
