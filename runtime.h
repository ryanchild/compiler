#define MM_SIZE 65538
#define R_SIZE 1024

extern size_t MM[MM_SIZE];
extern size_t R[R_SIZE];
extern const int SP;
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
