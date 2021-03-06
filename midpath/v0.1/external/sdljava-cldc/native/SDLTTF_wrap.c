/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 1.3.22
 * 
 * This file is not intended to be easily readable and contains a number of 
 * coding conventions designed to improve portability and efficiency. Do not make
 * changes to this file unless you know what you are doing--modify the SWIG 
 * interface file instead. 
 * ----------------------------------------------------------------------------- */


#if defined(__GNUC__)
    typedef long long __int64; /*For gcc on Windows */
#endif
#include <jni.h>
#include <stdlib.h>
#include <string.h>


/* Support for throwing Java exceptions */
typedef enum {
  SWIG_JavaOutOfMemoryError = 1, 
  SWIG_JavaIOException, 
  SWIG_JavaRuntimeException, 
  SWIG_JavaIndexOutOfBoundsException,
  SWIG_JavaArithmeticException,
  SWIG_JavaIllegalArgumentException,
  SWIG_JavaNullPointerException,
  SWIG_JavaDirectorPureVirtual,
  SWIG_JavaUnknownError
} SWIG_JavaExceptionCodes;

typedef struct {
  SWIG_JavaExceptionCodes code;
  const char *java_exception;
} SWIG_JavaExceptions_t;


static void SWIG_JavaThrowException(JNIEnv *jenv, SWIG_JavaExceptionCodes code, const char *msg) {
  jclass excep;
  static const SWIG_JavaExceptions_t java_exceptions[] = {
    { SWIG_JavaOutOfMemoryError, "java/lang/OutOfMemoryError" },
    { SWIG_JavaIOException, "java/io/IOException" },
    { SWIG_JavaRuntimeException, "java/lang/RuntimeException" },
    { SWIG_JavaIndexOutOfBoundsException, "java/lang/IndexOutOfBoundsException" },
    { SWIG_JavaArithmeticException, "java/lang/ArithmeticException" },
    { SWIG_JavaIllegalArgumentException, "java/lang/IllegalArgumentException" },
    { SWIG_JavaNullPointerException, "java/lang/NullPointerException" },
    { SWIG_JavaDirectorPureVirtual, "java/lang/RuntimeException" },
    { SWIG_JavaUnknownError,  "java/lang/UnknownError" },
    { (SWIG_JavaExceptionCodes)0,  "java/lang/UnknownError" } };
  const SWIG_JavaExceptions_t *except_ptr = java_exceptions;

  while (except_ptr->code != code && except_ptr->code)
    except_ptr++;

  (*jenv)->ExceptionClear(jenv);
  excep = (*jenv)->FindClass(jenv, except_ptr->java_exception);
  if (excep)
    (*jenv)->ThrowNew(jenv, excep, msg);
}


/* Contract support */

#define SWIG_contract_assert(nullreturn, expr, msg) if (!(expr)) {SWIG_JavaThrowException(jenv, SWIG_JavaIllegalArgumentException, msg); return nullreturn; } else


  #include "SDL_ttf.h"

  SDL_version SWIG_TTF_VERSION() {
    SDL_version version;

    TTF_VERSION(&version);

    return version;
  }

  SDL_Color color;
  SDL_Color back;

  SDL_Surface * TTF_RenderText_Solid_FAST(TTF_Font * font, const char* text, Uint8 r, Uint8 g, Uint8 b) {
    color.r = r;
    color.g = g;
    color.b = b;

    return TTF_RenderUTF8_Solid(font, text, color);
  }

  SDL_Surface * TTF_RenderText_Shaded_FAST(TTF_Font *font, const char *text, Uint8 fr, Uint8 fg, Uint8 fb, Uint8 br, Uint8 bg, Uint8 bb) {
    color.r = fr;
    color.g = fg;
    color.b = fb;

    back.r = br;
    back.g = bg;
    back.b = bb;

    return TTF_RenderUTF8_Shaded(font, text, color, back);
    
  }

  SDL_Surface * TTF_RenderText_Blended_FAST(TTF_Font *font, const char* text, Uint8 r, Uint8 g, Uint8 b) {
    color.r = r;
    color.g = g;
    color.b = b;

    return TTF_RenderUTF8_Blended(font, text, color);
  }

extern int TTF_Init(void);
extern TTF_Font *TTF_OpenFont(char const *,int);
extern TTF_Font *TTF_OpenFontIndex(char const *,int,long);
/*extern int TTF_GetFontStyle(TTF_Font *);*/
extern void TTF_SetFontStyle(TTF_Font *,int);
/*extern int TTF_FontHeight(TTF_Font *);
extern int TTF_FontAscent(TTF_Font *);
extern int TTF_FontDescent(TTF_Font *);
extern int TTF_FontLineSkip(TTF_Font *);
extern long TTF_FontFaces(TTF_Font *);
extern int TTF_FontFaceIsFixedWidth(TTF_Font *);
extern char *TTF_FontFaceFamilyName(TTF_Font *);
extern char *TTF_FontFaceStyleName(TTF_Font *);*/
extern int TTF_GlyphMetrics(TTF_Font *,Uint16,int *,int *,int *,int *,int *);
extern int TTF_SizeText(TTF_Font *,char const *,int *,int *);
extern int TTF_SizeUTF8(TTF_Font *,char const *,int *,int *);
extern int TTF_SizeUNICODE(TTF_Font *,Uint16 const *,int *,int *);
extern SDL_Surface *TTF_RenderText_Solid_FAST(TTF_Font *,char const *,Uint8,Uint8,Uint8);
extern SDL_Surface *TTF_RenderUTF8_Solid(TTF_Font *,char const *,SDL_Color);
extern SDL_Surface *TTF_RenderUNICODE_Solid(TTF_Font *,Uint16 const *,SDL_Color);
extern SDL_Surface *TTF_RenderText_Shaded_FAST(TTF_Font *,char const *,Uint8,Uint8,Uint8,Uint8,Uint8,Uint8);
extern SDL_Surface *TTF_RenderUTF8_Shaded(TTF_Font *,char const *,SDL_Color,SDL_Color);
extern SDL_Surface *TTF_RenderUNICODE_Shaded(TTF_Font *,Uint16 const *,SDL_Color,SDL_Color);
extern SDL_Surface *TTF_RenderGlyph_Shaded(TTF_Font *,Uint16,SDL_Color,SDL_Color);
extern SDL_Surface *TTF_RenderText_Blended_FAST(TTF_Font *,char const *,Uint8,Uint8,Uint8);
extern SDL_Surface *TTF_RenderUTF8_Blended(TTF_Font *,char const *,SDL_Color);
extern SDL_Surface *TTF_RenderUNICODE_Blended(TTF_Font *,Uint16 const *,SDL_Color);
extern SDL_Surface *TTF_RenderGlyph_Blended(TTF_Font *,Uint16,SDL_Color);
extern void TTF_CloseFont(TTF_Font *);
extern void TTF_Quit(void);
extern SDL_version SWIG_TTF_VERSION();

#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT jint JNICALL Java_sdljava_x_swig_SWIG_1SDLTTFJNI_TTF_1Init(JNIEnv *jenv, jclass jcls) {
    jint jresult = 0 ;
    int result;
    
    (void)jenv;
    (void)jcls;
    result = (int)TTF_Init();
    
    jresult = (jint)result; 
    return jresult;
}

JNIEXPORT jlong JNICALL Java_sdljava_x_swig_SWIG_1SDLTTFJNI_TTF_1OpenFontIndex(JNIEnv *jenv, jclass jcls, jstring jarg1, jint jarg2, jint jarg3) {
    jlong jresult = 0 ;
    char *arg1 ;
    int arg2 ;
    long arg3 ;
    TTF_Font *result;
    
    (void)jenv;
    (void)jcls;
    {
        arg1 = 0;
        if (jarg1) {
            arg1 = (char *)(*jenv)->GetStringUTFChars(jenv, jarg1, 0);
            if (!arg1) return 0;
        }
    }
    arg2 = (int)jarg2; 
    arg3 = (long)jarg3; 
    result = (TTF_Font *)TTF_OpenFontIndex((char const *)arg1,arg2,arg3);
    
    *(TTF_Font **)&jresult = result; 
    {
        if (arg1) (*jenv)->ReleaseStringUTFChars(jenv, jarg1, arg1); 
    }
    return jresult;
}


JNIEXPORT jlong JNICALL Java_sdljava_x_swig_SWIG_1SDLTTFJNI_TTF_1OpenFontIndexFromBuffer(JNIEnv *jenv, jclass jcls, jbyteArray dataArray, jint dataSize, jint ptsize, jint index) {
    
    jlong jresult = 0 ;
    TTF_Font *result;
    jint *data;
    
    data = (*jenv)->GetIntArrayElements(jenv, dataArray, 0);
    result = (TTF_Font *)TTF_OpenFontIndexRW(SDL_RWFromMem(data, dataSize), 1, ptsize, index);
    (*jenv)->ReleaseIntArrayElements(jenv, dataArray, data, 0);
    
    *(TTF_Font **)&jresult = result; 
    return jresult;
    
}


JNIEXPORT jint JNICALL Java_sdljava_x_swig_SWIG_1SDLTTFJNI_TTF_1GetFontStyle(JNIEnv *jenv, jclass jcls, jlong jarg1) {
    jint jresult = 0 ;
    TTF_Font *arg1 = (TTF_Font *) 0 ;
    int result;
    
    (void)jenv;
    (void)jcls;
    arg1 = *(TTF_Font **)&jarg1; 
    result = (int)TTF_GetFontStyle(arg1);
    
    jresult = (jint)result; 
    return jresult;
}


JNIEXPORT void JNICALL Java_sdljava_x_swig_SWIG_1SDLTTFJNI_TTF_1SetFontStyle(JNIEnv *jenv, jclass jcls, jlong jarg1, jint jarg2) {
    TTF_Font *arg1 = (TTF_Font *) 0 ;
    int arg2 ;
    
    (void)jenv;
    (void)jcls;
    arg1 = *(TTF_Font **)&jarg1; 
    arg2 = (int)jarg2; 
    TTF_SetFontStyle(arg1,arg2);
    
}


JNIEXPORT jint JNICALL Java_sdljava_x_swig_SWIG_1SDLTTFJNI_TTF_1FontHeight(JNIEnv *jenv, jclass jcls, jlong jarg1) {
    jint jresult = 0 ;
    TTF_Font *arg1 = (TTF_Font *) 0 ;
    int result;
    
    (void)jenv;
    (void)jcls;
    arg1 = *(TTF_Font **)&jarg1; 
    result = (int)TTF_FontHeight(arg1);
    
    jresult = (jint)result; 
    return jresult;
}


JNIEXPORT jint JNICALL Java_sdljava_x_swig_SWIG_1SDLTTFJNI_TTF_1FontAscent(JNIEnv *jenv, jclass jcls, jlong jarg1) {
    jint jresult = 0 ;
    TTF_Font *arg1 = (TTF_Font *) 0 ;
    int result;
    
    (void)jenv;
    (void)jcls;
    arg1 = *(TTF_Font **)&jarg1; 
    result = (int)TTF_FontAscent(arg1);
    
    jresult = (jint)result; 
    return jresult;
}


JNIEXPORT jint JNICALL Java_sdljava_x_swig_SWIG_1SDLTTFJNI_TTF_1FontDescent(JNIEnv *jenv, jclass jcls, jlong jarg1) {
    jint jresult = 0 ;
    TTF_Font *arg1 = (TTF_Font *) 0 ;
    int result;
    
    (void)jenv;
    (void)jcls;
    arg1 = *(TTF_Font **)&jarg1; 
    result = (int)TTF_FontDescent(arg1);
    
    jresult = (jint)result; 
    return jresult;
}


JNIEXPORT jint JNICALL Java_sdljava_x_swig_SWIG_1SDLTTFJNI_TTF_1FontLineSkip(JNIEnv *jenv, jclass jcls, jlong jarg1) {
    jint jresult = 0 ;
    TTF_Font *arg1 = (TTF_Font *) 0 ;
    int result;
    
    (void)jenv;
    (void)jcls;
    arg1 = *(TTF_Font **)&jarg1; 
    result = (int)TTF_FontLineSkip(arg1);
    
    jresult = (jint)result; 
    return jresult;
}


JNIEXPORT jint JNICALL Java_sdljava_x_swig_SWIG_1SDLTTFJNI_TTF_1FontFaces(JNIEnv *jenv, jclass jcls, jlong jarg1) {
    jint jresult = 0 ;
    TTF_Font *arg1 = (TTF_Font *) 0 ;
    long result;
    
    (void)jenv;
    (void)jcls;
    arg1 = *(TTF_Font **)&jarg1; 
    result = (long)TTF_FontFaces(arg1);
    
    jresult = (jint)result; 
    return jresult;
}


JNIEXPORT jint JNICALL Java_sdljava_x_swig_SWIG_1SDLTTFJNI_TTF_1FontFaceIsFixedWidth(JNIEnv *jenv, jclass jcls, jlong jarg1) {
    jint jresult = 0 ;
    TTF_Font *arg1 = (TTF_Font *) 0 ;
    int result;
    
    (void)jenv;
    (void)jcls;
    arg1 = *(TTF_Font **)&jarg1; 
    result = (int)TTF_FontFaceIsFixedWidth(arg1);
    
    jresult = (jint)result; 
    return jresult;
}


JNIEXPORT jstring JNICALL Java_sdljava_x_swig_SWIG_1SDLTTFJNI_TTF_1FontFaceFamilyName(JNIEnv *jenv, jclass jcls, jlong jarg1) {
    jstring jresult = 0 ;
    TTF_Font *arg1 = (TTF_Font *) 0 ;
    char *result;
    
    (void)jenv;
    (void)jcls;
    arg1 = *(TTF_Font **)&jarg1; 
    result = (char *)TTF_FontFaceFamilyName(arg1);
    
    {
        if(result) jresult = (*jenv)->NewStringUTF(jenv, result); 
    }
    return jresult;
}


JNIEXPORT jstring JNICALL Java_sdljava_x_swig_SWIG_1SDLTTFJNI_TTF_1FontFaceStyleName(JNIEnv *jenv, jclass jcls, jlong jarg1) {
    jstring jresult = 0 ;
    TTF_Font *arg1 = (TTF_Font *) 0 ;
    char *result;
    
    (void)jenv;
    (void)jcls;
    arg1 = *(TTF_Font **)&jarg1; 
    result = (char *)TTF_FontFaceStyleName(arg1);
    
    {
        if(result) jresult = (*jenv)->NewStringUTF(jenv, result); 
    }
    return jresult;
}


JNIEXPORT jint JNICALL Java_sdljava_x_swig_SWIG_1SDLTTFJNI_TTF_1GlyphMetrics(JNIEnv *jenv, jclass jcls, jlong jarg1, jint jarg2, jintArray jarg3, jintArray jarg4, jintArray jarg5, jintArray jarg6, jintArray jarg7) {
    jint jresult = 0 ;
    TTF_Font *arg1 = (TTF_Font *) 0 ;
    Uint16 arg2 ;
    int *arg3 = (int *) 0 ;
    int *arg4 = (int *) 0 ;
    int *arg5 = (int *) 0 ;
    int *arg6 = (int *) 0 ;
    int *arg7 = (int *) 0 ;
    int result;
    int temp3 ;
    int temp4 ;
    int temp5 ;
    int temp6 ;
    int temp7 ;
    
    (void)jenv;
    (void)jcls;
    arg1 = *(TTF_Font **)&jarg1; 
    arg2 = (Uint16)jarg2; 
    {
        if (!jarg3) {
            SWIG_JavaThrowException(jenv, SWIG_JavaNullPointerException, "array null");
            return 0;
        }
        if ((*jenv)->GetArrayLength(jenv, jarg3) == 0) {
            SWIG_JavaThrowException(jenv, SWIG_JavaIndexOutOfBoundsException, "Array must contain at least 1 element");
            return 0;
        }
        arg3 = &temp3; 
    }
    {
        if (!jarg4) {
            SWIG_JavaThrowException(jenv, SWIG_JavaNullPointerException, "array null");
            return 0;
        }
        if ((*jenv)->GetArrayLength(jenv, jarg4) == 0) {
            SWIG_JavaThrowException(jenv, SWIG_JavaIndexOutOfBoundsException, "Array must contain at least 1 element");
            return 0;
        }
        arg4 = &temp4; 
    }
    {
        if (!jarg5) {
            SWIG_JavaThrowException(jenv, SWIG_JavaNullPointerException, "array null");
            return 0;
        }
        if ((*jenv)->GetArrayLength(jenv, jarg5) == 0) {
            SWIG_JavaThrowException(jenv, SWIG_JavaIndexOutOfBoundsException, "Array must contain at least 1 element");
            return 0;
        }
        arg5 = &temp5; 
    }
    {
        if (!jarg6) {
            SWIG_JavaThrowException(jenv, SWIG_JavaNullPointerException, "array null");
            return 0;
        }
        if ((*jenv)->GetArrayLength(jenv, jarg6) == 0) {
            SWIG_JavaThrowException(jenv, SWIG_JavaIndexOutOfBoundsException, "Array must contain at least 1 element");
            return 0;
        }
        arg6 = &temp6; 
    }
    {
        if (!jarg7) {
            SWIG_JavaThrowException(jenv, SWIG_JavaNullPointerException, "array null");
            return 0;
        }
        if ((*jenv)->GetArrayLength(jenv, jarg7) == 0) {
            SWIG_JavaThrowException(jenv, SWIG_JavaIndexOutOfBoundsException, "Array must contain at least 1 element");
            return 0;
        }
        arg7 = &temp7; 
    }
    result = (int)TTF_GlyphMetrics(arg1,arg2,arg3,arg4,arg5,arg6,arg7);
    
    jresult = (jint)result; 
    {
        jint jvalue = (jint)temp3;
        (*jenv)->SetIntArrayRegion(jenv, jarg3, 0, 1, &jvalue);
    }
    {
        jint jvalue = (jint)temp4;
        (*jenv)->SetIntArrayRegion(jenv, jarg4, 0, 1, &jvalue);
    }
    {
        jint jvalue = (jint)temp5;
        (*jenv)->SetIntArrayRegion(jenv, jarg5, 0, 1, &jvalue);
    }
    {
        jint jvalue = (jint)temp6;
        (*jenv)->SetIntArrayRegion(jenv, jarg6, 0, 1, &jvalue);
    }
    {
        jint jvalue = (jint)temp7;
        (*jenv)->SetIntArrayRegion(jenv, jarg7, 0, 1, &jvalue);
    }
    return jresult;
}


JNIEXPORT jint JNICALL Java_sdljava_x_swig_SWIG_1SDLTTFJNI_TTF_1SizeText(JNIEnv *jenv, jclass jcls, jlong jarg1, jstring jarg2, jintArray jarg3, jintArray jarg4) {
    jint jresult = 0 ;
    TTF_Font *arg1 = (TTF_Font *) 0 ;
    char *arg2 ;
    int *arg3 = (int *) 0 ;
    int *arg4 = (int *) 0 ;
    int result;
    int temp3 ;
    int temp4 ;
    
    (void)jenv;
    (void)jcls;
    arg1 = *(TTF_Font **)&jarg1; 
    {
        arg2 = 0;
        if (jarg2) {
            arg2 = (char *)(*jenv)->GetStringUTFChars(jenv, jarg2, 0);
            if (!arg2) return 0;
        }
    }
    {
        if (!jarg3) {
            SWIG_JavaThrowException(jenv, SWIG_JavaNullPointerException, "array null");
            return 0;
        }
        if ((*jenv)->GetArrayLength(jenv, jarg3) == 0) {
            SWIG_JavaThrowException(jenv, SWIG_JavaIndexOutOfBoundsException, "Array must contain at least 1 element");
            return 0;
        }
        arg3 = &temp3; 
    }
    {
        if (!jarg4) {
            SWIG_JavaThrowException(jenv, SWIG_JavaNullPointerException, "array null");
            return 0;
        }
        if ((*jenv)->GetArrayLength(jenv, jarg4) == 0) {
            SWIG_JavaThrowException(jenv, SWIG_JavaIndexOutOfBoundsException, "Array must contain at least 1 element");
            return 0;
        }
        arg4 = &temp4; 
    }
    result = (int)TTF_SizeText(arg1,(char const *)arg2,arg3,arg4);
    
    jresult = (jint)result; 
    {
        jint jvalue = (jint)temp3;
        (*jenv)->SetIntArrayRegion(jenv, jarg3, 0, 1, &jvalue);
    }
    {
        jint jvalue = (jint)temp4;
        (*jenv)->SetIntArrayRegion(jenv, jarg4, 0, 1, &jvalue);
    }
    {
        if (arg2) (*jenv)->ReleaseStringUTFChars(jenv, jarg2, arg2); 
    }
    return jresult;
}


JNIEXPORT jint JNICALL Java_sdljava_x_swig_SWIG_1SDLTTFJNI_TTF_1SizeUTF8(JNIEnv *jenv, jclass jcls, jlong jarg1, jstring jarg2, jintArray jarg3, jintArray jarg4) {
    jint jresult = 0 ;
    TTF_Font *arg1 = (TTF_Font *) 0 ;
    char *arg2 ;
    int *arg3 = (int *) 0 ;
    int *arg4 = (int *) 0 ;
    int result;
    int temp3 ;
    int temp4 ;
    
    (void)jenv;
    (void)jcls;
    arg1 = *(TTF_Font **)&jarg1; 
    {
        arg2 = 0;
        if (jarg2) {
            arg2 = (char *)(*jenv)->GetStringUTFChars(jenv, jarg2, 0);
            if (!arg2) return 0;
        }
    }
    {
        if (!jarg3) {
            SWIG_JavaThrowException(jenv, SWIG_JavaNullPointerException, "array null");
            return 0;
        }
        if ((*jenv)->GetArrayLength(jenv, jarg3) == 0) {
            SWIG_JavaThrowException(jenv, SWIG_JavaIndexOutOfBoundsException, "Array must contain at least 1 element");
            return 0;
        }
        arg3 = &temp3; 
    }
    {
        if (!jarg4) {
            SWIG_JavaThrowException(jenv, SWIG_JavaNullPointerException, "array null");
            return 0;
        }
        if ((*jenv)->GetArrayLength(jenv, jarg4) == 0) {
            SWIG_JavaThrowException(jenv, SWIG_JavaIndexOutOfBoundsException, "Array must contain at least 1 element");
            return 0;
        }
        arg4 = &temp4; 
    }
    result = (int)TTF_SizeUTF8(arg1,(char const *)arg2,arg3,arg4);
    
    jresult = (jint)result; 
    {
        jint jvalue = (jint)temp3;
        (*jenv)->SetIntArrayRegion(jenv, jarg3, 0, 1, &jvalue);
    }
    {
        jint jvalue = (jint)temp4;
        (*jenv)->SetIntArrayRegion(jenv, jarg4, 0, 1, &jvalue);
    }
    {
        if (arg2) (*jenv)->ReleaseStringUTFChars(jenv, jarg2, arg2); 
    }
    return jresult;
}


JNIEXPORT jint JNICALL Java_sdljava_x_swig_SWIG_1SDLTTFJNI_TTF_1SizeUNICODE(JNIEnv *jenv, jclass jcls, jlong jarg1, jlong jarg2, jintArray jarg3, jintArray jarg4) {
    jint jresult = 0 ;
    TTF_Font *arg1 = (TTF_Font *) 0 ;
    Uint16 *arg2 = (Uint16 *) 0 ;
    int *arg3 = (int *) 0 ;
    int *arg4 = (int *) 0 ;
    int result;
    int temp3 ;
    int temp4 ;
    
    (void)jenv;
    (void)jcls;
    arg1 = *(TTF_Font **)&jarg1; 
    arg2 = *(Uint16 **)&jarg2; 
    {
        if (!jarg3) {
            SWIG_JavaThrowException(jenv, SWIG_JavaNullPointerException, "array null");
            return 0;
        }
        if ((*jenv)->GetArrayLength(jenv, jarg3) == 0) {
            SWIG_JavaThrowException(jenv, SWIG_JavaIndexOutOfBoundsException, "Array must contain at least 1 element");
            return 0;
        }
        arg3 = &temp3; 
    }
    {
        if (!jarg4) {
            SWIG_JavaThrowException(jenv, SWIG_JavaNullPointerException, "array null");
            return 0;
        }
        if ((*jenv)->GetArrayLength(jenv, jarg4) == 0) {
            SWIG_JavaThrowException(jenv, SWIG_JavaIndexOutOfBoundsException, "Array must contain at least 1 element");
            return 0;
        }
        arg4 = &temp4; 
    }
    result = (int)TTF_SizeUNICODE(arg1,(Uint16 const *)arg2,arg3,arg4);
    
    jresult = (jint)result; 
    {
        jint jvalue = (jint)temp3;
        (*jenv)->SetIntArrayRegion(jenv, jarg3, 0, 1, &jvalue);
    }
    {
        jint jvalue = (jint)temp4;
        (*jenv)->SetIntArrayRegion(jenv, jarg4, 0, 1, &jvalue);
    }
    return jresult;
}


JNIEXPORT jlong JNICALL Java_sdljava_x_swig_SWIG_1SDLTTFJNI_TTF_1RenderText_1Solid_1FAST(JNIEnv *jenv, jclass jcls, jlong jarg1, jstring jarg2, jshort jarg3, jshort jarg4, jshort jarg5) {
    jlong jresult = 0 ;
    TTF_Font *arg1 = (TTF_Font *) 0 ;
    char *arg2 ;
    Uint8 arg3 ;
    Uint8 arg4 ;
    Uint8 arg5 ;
    SDL_Surface *result;
    
    (void)jenv;
    (void)jcls;
    arg1 = *(TTF_Font **)&jarg1; 
    {
        arg2 = 0;
        if (jarg2) {
            arg2 = (char *)(*jenv)->GetStringUTFChars(jenv, jarg2, 0);
            if (!arg2) return 0;
        }
    }
    arg3 = (Uint8)jarg3; 
    arg4 = (Uint8)jarg4; 
    arg5 = (Uint8)jarg5; 
    result = (SDL_Surface *)TTF_RenderText_Solid_FAST(arg1,(char const *)arg2,arg3,arg4,arg5);
    
    *(SDL_Surface **)&jresult = result; 
    {
        if (arg2) (*jenv)->ReleaseStringUTFChars(jenv, jarg2, arg2); 
    }
    return jresult;
}


JNIEXPORT jlong JNICALL Java_sdljava_x_swig_SWIG_1SDLTTFJNI_TTF_1RenderUTF8_1Solid(JNIEnv *jenv, jclass jcls, jlong jarg1, jstring jarg2, jlong jarg3) {
    jlong jresult = 0 ;
    TTF_Font *arg1 = (TTF_Font *) 0 ;
    char *arg2 ;
    SDL_Color arg3 ;
    SDL_Surface *result;
    SDL_Color *argp3 ;
    
    (void)jenv;
    (void)jcls;
    arg1 = *(TTF_Font **)&jarg1; 
    {
        arg2 = 0;
        if (jarg2) {
            arg2 = (char *)(*jenv)->GetStringUTFChars(jenv, jarg2, 0);
            if (!arg2) return 0;
        }
    }
    argp3 = *(SDL_Color **)&jarg3; 
    if (!argp3) {
        SWIG_JavaThrowException(jenv, SWIG_JavaNullPointerException, "Attempt to dereference null SDL_Color");
        return 0;
    }
    arg3 = *argp3; 
    result = (SDL_Surface *)TTF_RenderUTF8_Solid(arg1,(char const *)arg2,arg3);
    
    *(SDL_Surface **)&jresult = result; 
    {
        if (arg2) (*jenv)->ReleaseStringUTFChars(jenv, jarg2, arg2); 
    }
    return jresult;
}


JNIEXPORT jlong JNICALL Java_sdljava_x_swig_SWIG_1SDLTTFJNI_TTF_1RenderUNICODE_1Solid(JNIEnv *jenv, jclass jcls, jlong jarg1, jlong jarg2, jlong jarg3) {
    jlong jresult = 0 ;
    TTF_Font *arg1 = (TTF_Font *) 0 ;
    Uint16 *arg2 = (Uint16 *) 0 ;
    SDL_Color arg3 ;
    SDL_Surface *result;
    SDL_Color *argp3 ;
    
    (void)jenv;
    (void)jcls;
    arg1 = *(TTF_Font **)&jarg1; 
    arg2 = *(Uint16 **)&jarg2; 
    argp3 = *(SDL_Color **)&jarg3; 
    if (!argp3) {
        SWIG_JavaThrowException(jenv, SWIG_JavaNullPointerException, "Attempt to dereference null SDL_Color");
        return 0;
    }
    arg3 = *argp3; 
    result = (SDL_Surface *)TTF_RenderUNICODE_Solid(arg1,(Uint16 const *)arg2,arg3);
    
    *(SDL_Surface **)&jresult = result; 
    return jresult;
}


JNIEXPORT jlong JNICALL Java_sdljava_x_swig_SWIG_1SDLTTFJNI_TTF_1RenderText_1Shaded_1FAST(JNIEnv *jenv, jclass jcls, jlong jarg1, jstring jarg2, jshort jarg3, jshort jarg4, jshort jarg5, jshort jarg6, jshort jarg7, jshort jarg8) {
    jlong jresult = 0 ;
    TTF_Font *arg1 = (TTF_Font *) 0 ;
    char *arg2 ;
    Uint8 arg3 ;
    Uint8 arg4 ;
    Uint8 arg5 ;
    Uint8 arg6 ;
    Uint8 arg7 ;
    Uint8 arg8 ;
    SDL_Surface *result;
    
    (void)jenv;
    (void)jcls;
    arg1 = *(TTF_Font **)&jarg1; 
    {
        arg2 = 0;
        if (jarg2) {
            arg2 = (char *)(*jenv)->GetStringUTFChars(jenv, jarg2, 0);
            if (!arg2) return 0;
        }
    }
    arg3 = (Uint8)jarg3; 
    arg4 = (Uint8)jarg4; 
    arg5 = (Uint8)jarg5; 
    arg6 = (Uint8)jarg6; 
    arg7 = (Uint8)jarg7; 
    arg8 = (Uint8)jarg8; 
    result = (SDL_Surface *)TTF_RenderText_Shaded_FAST(arg1,(char const *)arg2,arg3,arg4,arg5,arg6,arg7,arg8);
    
    *(SDL_Surface **)&jresult = result; 
    {
        if (arg2) (*jenv)->ReleaseStringUTFChars(jenv, jarg2, arg2); 
    }
    return jresult;
}


JNIEXPORT jlong JNICALL Java_sdljava_x_swig_SWIG_1SDLTTFJNI_TTF_1RenderUTF8_1Shaded(JNIEnv *jenv, jclass jcls, jlong jarg1, jstring jarg2, jlong jarg3, jlong jarg4) {
    jlong jresult = 0 ;
    TTF_Font *arg1 = (TTF_Font *) 0 ;
    char *arg2 ;
    SDL_Color arg3 ;
    SDL_Color arg4 ;
    SDL_Surface *result;
    SDL_Color *argp3 ;
    SDL_Color *argp4 ;
    
    (void)jenv;
    (void)jcls;
    arg1 = *(TTF_Font **)&jarg1; 
    {
        arg2 = 0;
        if (jarg2) {
            arg2 = (char *)(*jenv)->GetStringUTFChars(jenv, jarg2, 0);
            if (!arg2) return 0;
        }
    }
    argp3 = *(SDL_Color **)&jarg3; 
    if (!argp3) {
        SWIG_JavaThrowException(jenv, SWIG_JavaNullPointerException, "Attempt to dereference null SDL_Color");
        return 0;
    }
    arg3 = *argp3; 
    argp4 = *(SDL_Color **)&jarg4; 
    if (!argp4) {
        SWIG_JavaThrowException(jenv, SWIG_JavaNullPointerException, "Attempt to dereference null SDL_Color");
        return 0;
    }
    arg4 = *argp4; 
    result = (SDL_Surface *)TTF_RenderUTF8_Shaded(arg1,(char const *)arg2,arg3,arg4);
    
    *(SDL_Surface **)&jresult = result; 
    {
        if (arg2) (*jenv)->ReleaseStringUTFChars(jenv, jarg2, arg2); 
    }
    return jresult;
}


JNIEXPORT jlong JNICALL Java_sdljava_x_swig_SWIG_1SDLTTFJNI_TTF_1RenderUNICODE_1Shaded(JNIEnv *jenv, jclass jcls, jlong jarg1, jlong jarg2, jlong jarg3, jlong jarg4) {
    jlong jresult = 0 ;
    TTF_Font *arg1 = (TTF_Font *) 0 ;
    Uint16 *arg2 = (Uint16 *) 0 ;
    SDL_Color arg3 ;
    SDL_Color arg4 ;
    SDL_Surface *result;
    SDL_Color *argp3 ;
    SDL_Color *argp4 ;
    
    (void)jenv;
    (void)jcls;
    arg1 = *(TTF_Font **)&jarg1; 
    arg2 = *(Uint16 **)&jarg2; 
    argp3 = *(SDL_Color **)&jarg3; 
    if (!argp3) {
        SWIG_JavaThrowException(jenv, SWIG_JavaNullPointerException, "Attempt to dereference null SDL_Color");
        return 0;
    }
    arg3 = *argp3; 
    argp4 = *(SDL_Color **)&jarg4; 
    if (!argp4) {
        SWIG_JavaThrowException(jenv, SWIG_JavaNullPointerException, "Attempt to dereference null SDL_Color");
        return 0;
    }
    arg4 = *argp4; 
    result = (SDL_Surface *)TTF_RenderUNICODE_Shaded(arg1,(Uint16 const *)arg2,arg3,arg4);
    
    *(SDL_Surface **)&jresult = result; 
    return jresult;
}


JNIEXPORT jlong JNICALL Java_sdljava_x_swig_SWIG_1SDLTTFJNI_TTF_1RenderGlyph_1Shaded(JNIEnv *jenv, jclass jcls, jlong jarg1, jint jarg2, jlong jarg3, jlong jarg4) {
    jlong jresult = 0 ;
    TTF_Font *arg1 = (TTF_Font *) 0 ;
    Uint16 arg2 ;
    SDL_Color arg3 ;
    SDL_Color arg4 ;
    SDL_Surface *result;
    SDL_Color *argp3 ;
    SDL_Color *argp4 ;
    
    (void)jenv;
    (void)jcls;
    arg1 = *(TTF_Font **)&jarg1; 
    arg2 = (Uint16)jarg2; 
    argp3 = *(SDL_Color **)&jarg3; 
    if (!argp3) {
        SWIG_JavaThrowException(jenv, SWIG_JavaNullPointerException, "Attempt to dereference null SDL_Color");
        return 0;
    }
    arg3 = *argp3; 
    argp4 = *(SDL_Color **)&jarg4; 
    if (!argp4) {
        SWIG_JavaThrowException(jenv, SWIG_JavaNullPointerException, "Attempt to dereference null SDL_Color");
        return 0;
    }
    arg4 = *argp4; 
    result = (SDL_Surface *)TTF_RenderGlyph_Shaded(arg1,arg2,arg3,arg4);
    
    *(SDL_Surface **)&jresult = result; 
    return jresult;
}


JNIEXPORT jlong JNICALL Java_sdljava_x_swig_SWIG_1SDLTTFJNI_TTF_1RenderText_1Blended_1FAST(JNIEnv *jenv, jclass jcls, jlong jarg1, jstring jarg2, jshort jarg3, jshort jarg4, jshort jarg5) {
    jlong jresult = 0 ;
    TTF_Font *arg1 = (TTF_Font *) 0 ;
    char *arg2 ;
    Uint8 arg3 ;
    Uint8 arg4 ;
    Uint8 arg5 ;
    SDL_Surface *result;
    
    (void)jenv;
    (void)jcls;
    arg1 = *(TTF_Font **)&jarg1; 
    {
        arg2 = 0;
        if (jarg2) {
            arg2 = (char *)(*jenv)->GetStringUTFChars(jenv, jarg2, 0);
            if (!arg2) return 0;
        }
    }
    arg3 = (Uint8)jarg3; 
    arg4 = (Uint8)jarg4; 
    arg5 = (Uint8)jarg5; 
    result = (SDL_Surface *)TTF_RenderText_Blended_FAST(arg1,(char const *)arg2,arg3,arg4,arg5);
    
    *(SDL_Surface **)&jresult = result; 
    {
        if (arg2) (*jenv)->ReleaseStringUTFChars(jenv, jarg2, arg2); 
    }
    return jresult;
}


JNIEXPORT jlong JNICALL Java_sdljava_x_swig_SWIG_1SDLTTFJNI_TTF_1RenderUTF8_1Blended(JNIEnv *jenv, jclass jcls, jlong jarg1, jstring jarg2, jlong jarg3) {
    jlong jresult = 0 ;
    TTF_Font *arg1 = (TTF_Font *) 0 ;
    char *arg2 ;
    SDL_Color arg3 ;
    SDL_Surface *result;
    SDL_Color *argp3 ;
    
    (void)jenv;
    (void)jcls;
    arg1 = *(TTF_Font **)&jarg1; 
    {
        arg2 = 0;
        if (jarg2) {
            arg2 = (char *)(*jenv)->GetStringUTFChars(jenv, jarg2, 0);
            if (!arg2) return 0;
        }
    }
    argp3 = *(SDL_Color **)&jarg3; 
    if (!argp3) {
        SWIG_JavaThrowException(jenv, SWIG_JavaNullPointerException, "Attempt to dereference null SDL_Color");
        return 0;
    }
    arg3 = *argp3; 
    result = (SDL_Surface *)TTF_RenderUTF8_Blended(arg1,(char const *)arg2,arg3);
    
    *(SDL_Surface **)&jresult = result; 
    {
        if (arg2) (*jenv)->ReleaseStringUTFChars(jenv, jarg2, arg2); 
    }
    return jresult;
}


JNIEXPORT jlong JNICALL Java_sdljava_x_swig_SWIG_1SDLTTFJNI_TTF_1RenderUNICODE_1Blended(JNIEnv *jenv, jclass jcls, jlong jarg1, jlong jarg2, jlong jarg3) {
    jlong jresult = 0 ;
    TTF_Font *arg1 = (TTF_Font *) 0 ;
    Uint16 *arg2 = (Uint16 *) 0 ;
    SDL_Color arg3 ;
    SDL_Surface *result;
    SDL_Color *argp3 ;
    
    (void)jenv;
    (void)jcls;
    arg1 = *(TTF_Font **)&jarg1; 
    arg2 = *(Uint16 **)&jarg2; 
    argp3 = *(SDL_Color **)&jarg3; 
    if (!argp3) {
        SWIG_JavaThrowException(jenv, SWIG_JavaNullPointerException, "Attempt to dereference null SDL_Color");
        return 0;
    }
    arg3 = *argp3; 
    result = (SDL_Surface *)TTF_RenderUNICODE_Blended(arg1,(Uint16 const *)arg2,arg3);
    
    *(SDL_Surface **)&jresult = result; 
    return jresult;
}


JNIEXPORT jlong JNICALL Java_sdljava_x_swig_SWIG_1SDLTTFJNI_TTF_1RenderGlyph_1Blended(JNIEnv *jenv, jclass jcls, jlong jarg1, jint jarg2, jlong jarg3) {
    jlong jresult = 0 ;
    TTF_Font *arg1 = (TTF_Font *) 0 ;
    Uint16 arg2 ;
    SDL_Color arg3 ;
    SDL_Surface *result;
    SDL_Color *argp3 ;
    
    (void)jenv;
    (void)jcls;
    arg1 = *(TTF_Font **)&jarg1; 
    arg2 = (Uint16)jarg2; 
    argp3 = *(SDL_Color **)&jarg3; 
    if (!argp3) {
        SWIG_JavaThrowException(jenv, SWIG_JavaNullPointerException, "Attempt to dereference null SDL_Color");
        return 0;
    }
    arg3 = *argp3; 
    result = (SDL_Surface *)TTF_RenderGlyph_Blended(arg1,arg2,arg3);
    
    *(SDL_Surface **)&jresult = result; 
    return jresult;
}


JNIEXPORT void JNICALL Java_sdljava_x_swig_SWIG_1SDLTTFJNI_TTF_1CloseFont(JNIEnv *jenv, jclass jcls, jlong jarg1) {
    TTF_Font *arg1 = (TTF_Font *) 0 ;
    
    (void)jenv;
    (void)jcls;
    arg1 = *(TTF_Font **)&jarg1; 
    TTF_CloseFont(arg1);
    
}


JNIEXPORT void JNICALL Java_sdljava_x_swig_SWIG_1SDLTTFJNI_TTF_1Quit(JNIEnv *jenv, jclass jcls) {
    (void)jenv;
    (void)jcls;
    TTF_Quit();
    
}


JNIEXPORT jlong JNICALL Java_sdljava_x_swig_SWIG_1SDLTTFJNI_SWIG_1TTF_1VERSION(JNIEnv *jenv, jclass jcls) {
    jlong jresult = 0 ;
    SDL_version result;
    
    (void)jenv;
    (void)jcls;
    result = SWIG_TTF_VERSION();
    
    {
        SDL_version * resultptr = (SDL_version *) malloc(sizeof(SDL_version));
        memmove(resultptr, &result, sizeof(SDL_version));
        *(SDL_version **)&jresult = resultptr;
    }
    return jresult;
}


#ifdef __cplusplus
}
#endif

