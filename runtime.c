#include "stdio.h"
#include "math.h"

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

int sqrt(int i)
{
  return sqrt(i);
}
