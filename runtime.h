#define MM_SIZE 65538
#define STACK_START 1024
#define R_SIZE 1024

extern int MM[MM_SIZE];
extern int R[R_SIZE];
extern const int SP;
extern const int FP;
extern float TMP_FLOAT;
extern char TMP_STRING[256];

extern bool getBool();
extern int getInt();
extern float getFloat();
extern void getString(char*);
extern int putBool(bool);
extern int putInt(int);
extern int putFloat(float);
extern int putString(const char*);
