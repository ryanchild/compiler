#include "stdbool.h"
#include "stdio.h"
#include "math.h"
#include "runtime.h"

void* MM[MM_SIZE];
void* R[R_SIZE];
int SP;
int TMP_INT;
int TMP_FLOAT;
char TMP_STRING[256];

bool getBool()
{
  int a;
  scanf("%d", &a);
  return a;
}

int getInt()
{
  int a;
  scanf("%d", &a);
  return a;
}

float getFloat()
{
  float a;
  scanf("%f", &a);
  return a;
}

void getString(char* s)
{
  scanf("%s",s);
}

int putBool(bool b)
{
  printf("%s", b ? "true" : "false");
  return 0;
}

int putInt(int i)
{
  printf("%d",i);
  return 0;
}

int putFloat(float f)
{
  printf("%f",f);
  return 0;
}

int putString(const char* s)
{
  printf("%s",s);
  return 0;
}
