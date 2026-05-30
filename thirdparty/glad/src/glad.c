#include <glad/glad.h>
#include <GLFW/glfw3.h>

PFNGLCLEARCOLORPROC glClearColor;
PFNGLCLEARPROC glClear;
PFNGLVIEWPORTPROC glViewport;
PFNGLENABLEPROC glEnable;
PFNGLENABLEPROC glDisable;
PFNGLBLENDFUNCPROC glBlendFunc;
PFNGLPOLYGONMODEPROC glPolygonMode;
PFNGLBINDTEXTUREPROC glBindTexture;
PFNGLGENTEXTURESPROC glGenTextures;
PFNGLDELETETEXTURESPROC glDeleteTextures;
PFNGLTEXIMAGE2DPROC glTexImage2D;
PFNGLTEXPARAMETERIPROC glTexParameteri;
PFNGLDRAWELEMENTSPROC glDrawElements;
PFNGLDRAWARRAYSPROC glDrawArrays;
PFNGLGETSTRINGPROC glGetString;
PFNGLGETINTEGERVPROC glGetIntegerv;
PFNGLDEPTHFUNCPROC glDepthFunc;
PFNGLDEPTHMASKPROC glDepthMask;
PFNGLCULLFACEPROC glCullFace;
PFNGLFRONTFACEPROC glFrontFace;
PFNGLCREATESHADERPROC glCreateShader;
PFNGLSHADERSOURCEPROC glShaderSource;
PFNGLCOMPILESHADERPROC glCompileShader;
PFNGLGETSHADERIVPROC glGetShaderiv;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
PFNGLDELETESHADERPROC glDeleteShader;
PFNGLCREATEPROGRAMPROC glCreateProgram;
PFNGLATTACHSHADERPROC glAttachShader;
PFNGLLINKPROGRAMPROC glLinkProgram;
PFNGLGETPROGRAMIVPROC glGetProgramiv;
PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
PFNGLDELETEPROGRAMPROC glDeleteProgram;
PFNGLUSEPROGRAMPROC glUseProgram;
PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
PFNGLUNIFORM1IPROC glUniform1i;
PFNGLUNIFORM1FPROC glUniform1f;
PFNGLUNIFORM2FVPROC glUniform2fv;
PFNGLUNIFORM3FVPROC glUniform3fv;
PFNGLUNIFORM4FVPROC glUniform4fv;
PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;
PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays;
PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
PFNGLGENBUFFERSPROC glGenBuffers;
PFNGLDELETEBUFFERSPROC glDeleteBuffers;
PFNGLBINDBUFFERPROC glBindBuffer;
PFNGLBUFFERDATAPROC glBufferData;
PFNGLBUFFERSUBDATAPROC glBufferSubData;
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
PFNGLACTIVETEXTUREPROC glActiveTexture;
PFNGLGENERATEMIPMAPPROC glGenerateMipmap;
PFNGLGENFRAMEBUFFERSPROC glad_glGenFramebuffers;
PFNGLBINDFRAMEBUFFERPROC glad_glBindFramebuffer;
PFNGLDELETEFRAMEBUFFERSPROC glad_glDeleteFramebuffers;
PFNGLFRAMEBUFFERTEXTURE2DPROC glad_glFramebufferTexture2D;
PFNGLFRAMEBUFFERRENDERBUFFERPROC glad_glFramebufferRenderbuffer;
PFNGLCHECKFRAMEBUFFERSTATUSPROC glad_glCheckFramebufferStatus;
PFNGLGENRENDERBUFFERSPROC glad_glGenRenderbuffers;
PFNGLBINDRENDERBUFFERPROC glad_glBindRenderbuffer;
PFNGLDELETERENDERBUFFERSPROC glad_glDeleteRenderbuffers;
PFNGLRENDERBUFFERSTORAGEPROC glad_glRenderbufferStorage;
PFNGLBLITFRAMEBUFFERPROC glad_glBlitFramebuffer;
PFNGLGETFLOATVPROC glad_glGetFloatv;

static void* (*gl_loader)(const char* name);

static void* load_proc(const char* name) {
    void* ptr = gl_loader(name);
    return ptr;
}

int gladLoadGLLoader(void* (*load)(const char*)) {
    if (!load) return 0;
    gl_loader = load;

    glClearColor = (PFNGLCLEARCOLORPROC)load_proc("glClearColor");
    glClear = (PFNGLCLEARPROC)load_proc("glClear");
    glViewport = (PFNGLVIEWPORTPROC)load_proc("glViewport");
    glEnable = (PFNGLENABLEPROC)load_proc("glEnable");
    glDisable = (PFNGLENABLEPROC)load_proc("glDisable");
    glBlendFunc = (PFNGLBLENDFUNCPROC)load_proc("glBlendFunc");
    glPolygonMode = (PFNGLPOLYGONMODEPROC)load_proc("glPolygonMode");
    glBindTexture = (PFNGLBINDTEXTUREPROC)load_proc("glBindTexture");
    glGenTextures = (PFNGLGENTEXTURESPROC)load_proc("glGenTextures");
    glDeleteTextures = (PFNGLDELETETEXTURESPROC)load_proc("glDeleteTextures");
    glTexImage2D = (PFNGLTEXIMAGE2DPROC)load_proc("glTexImage2D");
    glTexParameteri = (PFNGLTEXPARAMETERIPROC)load_proc("glTexParameteri");
    glDrawElements = (PFNGLDRAWELEMENTSPROC)load_proc("glDrawElements");
    glDrawArrays = (PFNGLDRAWARRAYSPROC)load_proc("glDrawArrays");
    glGetString = (PFNGLGETSTRINGPROC)load_proc("glGetString");
    glGetIntegerv = (PFNGLGETINTEGERVPROC)load_proc("glGetIntegerv");
    glDepthFunc = (PFNGLDEPTHFUNCPROC)load_proc("glDepthFunc");
    glDepthMask = (PFNGLDEPTHMASKPROC)load_proc("glDepthMask");
    glCullFace = (PFNGLCULLFACEPROC)load_proc("glCullFace");
    glFrontFace = (PFNGLFRONTFACEPROC)load_proc("glFrontFace");
    glCreateShader = (PFNGLCREATESHADERPROC)load_proc("glCreateShader");
    glShaderSource = (PFNGLSHADERSOURCEPROC)load_proc("glShaderSource");
    glCompileShader = (PFNGLCOMPILESHADERPROC)load_proc("glCompileShader");
    glGetShaderiv = (PFNGLGETSHADERIVPROC)load_proc("glGetShaderiv");
    glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)load_proc("glGetShaderInfoLog");
    glDeleteShader = (PFNGLDELETESHADERPROC)load_proc("glDeleteShader");
    glCreateProgram = (PFNGLCREATEPROGRAMPROC)load_proc("glCreateProgram");
    glAttachShader = (PFNGLATTACHSHADERPROC)load_proc("glAttachShader");
    glLinkProgram = (PFNGLLINKPROGRAMPROC)load_proc("glLinkProgram");
    glGetProgramiv = (PFNGLGETPROGRAMIVPROC)load_proc("glGetProgramiv");
    glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)load_proc("glGetProgramInfoLog");
    glDeleteProgram = (PFNGLDELETEPROGRAMPROC)load_proc("glDeleteProgram");
    glUseProgram = (PFNGLUSEPROGRAMPROC)load_proc("glUseProgram");
    glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)load_proc("glGetUniformLocation");
    glUniform1i = (PFNGLUNIFORM1IPROC)load_proc("glUniform1i");
    glUniform1f = (PFNGLUNIFORM1FPROC)load_proc("glUniform1f");
    glUniform2fv = (PFNGLUNIFORM2FVPROC)load_proc("glUniform2fv");
    glUniform3fv = (PFNGLUNIFORM3FVPROC)load_proc("glUniform3fv");
    glUniform4fv = (PFNGLUNIFORM4FVPROC)load_proc("glUniform4fv");
    glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC)load_proc("glUniformMatrix4fv");
    glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC)load_proc("glGenVertexArrays");
    glDeleteVertexArrays = (PFNGLDELETEVERTEXARRAYSPROC)load_proc("glDeleteVertexArrays");
    glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)load_proc("glBindVertexArray");
    glGenBuffers = (PFNGLGENBUFFERSPROC)load_proc("glGenBuffers");
    glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)load_proc("glDeleteBuffers");
    glBindBuffer = (PFNGLBINDBUFFERPROC)load_proc("glBindBuffer");
    glBufferData = (PFNGLBUFFERDATAPROC)load_proc("glBufferData");
    glBufferSubData = (PFNGLBUFFERSUBDATAPROC)load_proc("glBufferSubData");
    glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)load_proc("glEnableVertexAttribArray");
    glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)load_proc("glVertexAttribPointer");
    glActiveTexture = (PFNGLACTIVETEXTUREPROC)load_proc("glActiveTexture");
    glGenerateMipmap = (PFNGLGENERATEMIPMAPPROC)load_proc("glGenerateMipmap");
    glad_glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC)load_proc("glGenFramebuffers");
    glad_glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC)load_proc("glBindFramebuffer");
    glad_glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC)load_proc("glDeleteFramebuffers");
    glad_glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC)load_proc("glFramebufferTexture2D");
    glad_glFramebufferRenderbuffer = (PFNGLFRAMEBUFFERRENDERBUFFERPROC)load_proc("glFramebufferRenderbuffer");
    glad_glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC)load_proc("glCheckFramebufferStatus");
    glad_glGenRenderbuffers = (PFNGLGENRENDERBUFFERSPROC)load_proc("glGenRenderbuffers");
    glad_glBindRenderbuffer = (PFNGLBINDRENDERBUFFERPROC)load_proc("glBindRenderbuffer");
    glad_glDeleteRenderbuffers = (PFNGLDELETERENDERBUFFERSPROC)load_proc("glDeleteRenderbuffers");
    glad_glRenderbufferStorage = (PFNGLRENDERBUFFERSTORAGEPROC)load_proc("glRenderbufferStorage");
    glad_glBlitFramebuffer = (PFNGLBLITFRAMEBUFFERPROC)load_proc("glBlitFramebuffer");
    glad_glGetFloatv = (PFNGLGETFLOATVPROC)load_proc("glGetFloatv");

    return 1;
}

int gladLoadGL(void) {
    return gladLoadGLLoader((void* (*)(const char*))glfwGetProcAddress);
}
