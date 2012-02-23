#define MM_SIZE 65538
#define R_SIZE 1024

extern void* MM[MM_SIZE];
extern void* R[R_SIZE];
extern int SP;
extern int TMP_INT;
extern int TMP_FLOAT;
extern char TMP_STRING[256];

extern bool getBool();
extern int getInt();
extern float getFloat();
extern void getString(char*);
extern int putBool(bool);
extern int putInt(int);
extern int putFloat(float);
extern int putString(const char*);
