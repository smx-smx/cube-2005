// gles1compat.h: the subset of desktop OpenGL 1.x and GLU used by cube, on OpenGL ES 1.1
//
// - immediate mode (glBegin/glEnd, GL_QUADS/GL_QUAD_STRIP/GL_POLYGON) drawn with client arrays
// - display lists recording immediate mode geometry (that's all cube puts in them)
// - glPolygonMode(GL_LINE) for immediate mode polygons
// - double precision variants, glOrtho, glClearDepth
// - the GLU functions cube uses: perspective, unproject, sphere quadric, mipmaps, image scaling

#ifndef GLES1COMPAT_H
#define GLES1COMPAT_H

#include <GLES/gl.h>
#include <GLES/glext.h>

// desktop GL enums missing from GLES 1.1
#define GL_QUADS                0x0007
#define GL_QUAD_STRIP           0x0008
#define GL_POLYGON              0x0009
#define GL_LINE                 0x1B01
#define GL_FILL                 0x1B02
#define GL_POLYGON_OFFSET_LINE  0x2A02
#define GL_COMPILE              0x1300
#define GL_DEPTH_COMPONENT      0x1902
#define GL_COMBINE_EXT          GL_COMBINE
#define GL_COMBINE_RGB_EXT      GL_COMBINE_RGB
#define GL_RGB_SCALE_EXT        GL_RGB_SCALE
#define GL_SOURCE0_RGB_EXT      GL_SRC0_RGB
#define GL_SOURCE1_RGB_EXT      GL_SRC1_RGB
#define GL_PRIMARY_COLOR_EXT    GL_PRIMARY_COLOR

typedef double GLdouble;
typedef double GLclampd;

void cgl_Begin(GLenum mode);
void cgl_End();
void cgl_Vertex3f(GLfloat x, GLfloat y, GLfloat z);
void cgl_TexCoord2f(GLfloat s, GLfloat t);
void cgl_Color4f(GLfloat r, GLfloat g, GLfloat b, GLfloat a);
GLuint cgl_GenLists(GLsizei range);
void cgl_NewList(GLuint list, GLenum mode);
void cgl_EndList();
void cgl_CallList(GLuint list);
void cgl_PolygonMode(GLenum face, GLenum mode);
void cgl_Enable(GLenum cap);
void cgl_Disable(GLenum cap);
void cgl_VertexPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *ptr);
void cgl_ColorPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *ptr);
void cgl_TexCoordPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *ptr);
void cgl_EnableClientState(GLenum array);
void cgl_DisableClientState(GLenum array);
void cgl_ReadPixels(GLint x, GLint y, GLsizei w, GLsizei h, GLenum format, GLenum type, GLvoid *pixels);
void cgl_LoadMatrixd(const GLdouble *m);
void cgl_GetDoublev(GLenum pname, GLdouble *params);

// gles1compat.cpp calls the real functions
#ifndef GLES1COMPAT_IMPL
#define glBegin                 cgl_Begin
#define glEnd                   cgl_End
#define glVertex2i(x, y)        cgl_Vertex3f((GLfloat)(x), (GLfloat)(y), 0)
#define glVertex2f(x, y)        cgl_Vertex3f((GLfloat)(x), (GLfloat)(y), 0)
#define glVertex3f(x, y, z)     cgl_Vertex3f((GLfloat)(x), (GLfloat)(y), (GLfloat)(z))
#define glVertex3d(x, y, z)     cgl_Vertex3f((GLfloat)(x), (GLfloat)(y), (GLfloat)(z))
#define glTexCoord2f(s, t)      cgl_TexCoord2f((GLfloat)(s), (GLfloat)(t))
#define glTexCoord2d(s, t)      cgl_TexCoord2f((GLfloat)(s), (GLfloat)(t))
#define glColor3ub(r, g, b)     cgl_Color4f((r)/255.0f, (g)/255.0f, (b)/255.0f, 1.0f)
#define glColor3d(r, g, b)      cgl_Color4f((GLfloat)(r), (GLfloat)(g), (GLfloat)(b), 1.0f)
#define glColor3f(r, g, b)      cgl_Color4f((GLfloat)(r), (GLfloat)(g), (GLfloat)(b), 1.0f)
#define glColor3fv(v)           cgl_Color4f((v)[0], (v)[1], (v)[2], 1.0f)
#define glColor4f(r, g, b, a)   cgl_Color4f((GLfloat)(r), (GLfloat)(g), (GLfloat)(b), (GLfloat)(a))
#define glGenLists              cgl_GenLists
#define glNewList               cgl_NewList
#define glEndList               cgl_EndList
#define glCallList              cgl_CallList
#define glPolygonMode           cgl_PolygonMode
#define glEnable                cgl_Enable
#define glDisable               cgl_Disable
#define glVertexPointer         cgl_VertexPointer
#define glColorPointer          cgl_ColorPointer
#define glTexCoordPointer       cgl_TexCoordPointer
#define glEnableClientState     cgl_EnableClientState
#define glDisableClientState    cgl_DisableClientState
#define glReadPixels            cgl_ReadPixels
#define glLoadMatrixd           cgl_LoadMatrixd
#define glGetDoublev            cgl_GetDoublev
#define glRotated(a, x, y, z)   glRotatef((GLfloat)(a), (GLfloat)(x), (GLfloat)(y), (GLfloat)(z))
#define glTranslated(x, y, z)   glTranslatef((GLfloat)(x), (GLfloat)(y), (GLfloat)(z))
#define glClearDepth(d)         glClearDepthf((GLclampf)(d))
#define glOrtho(l, r, b, t, n, f) glOrthof((GLfloat)(l), (GLfloat)(r), (GLfloat)(b), (GLfloat)(t), (GLfloat)(n), (GLfloat)(f))
#define glFogi(p, v)            glFogf((p), (GLfloat)(v))
#endif

// GLU
#define GLU_FILL    100012
#define GLU_INSIDE  100021
struct GLUquadric { int inside, textured; };
typedef GLUquadric GLUquadricObj;

void gluPerspective(GLdouble fovy, GLdouble aspect, GLdouble znear, GLdouble zfar);
GLint gluUnProject(GLdouble winx, GLdouble winy, GLdouble winz, const GLdouble model[16], const GLdouble proj[16],
                   const GLint viewport[4], GLdouble *objx, GLdouble *objy, GLdouble *objz);
GLUquadric *gluNewQuadric();
void gluDeleteQuadric(GLUquadric *q);
void gluQuadricDrawStyle(GLUquadric *q, GLenum style);
void gluQuadricOrientation(GLUquadric *q, GLenum orientation);
void gluQuadricTexture(GLUquadric *q, GLboolean textured);
void gluSphere(GLUquadric *q, GLdouble radius, GLint slices, GLint stacks);
GLint gluBuild2DMipmaps(GLenum target, GLint components, GLint w, GLint h, GLenum format, GLenum type, const void *data);
GLint gluScaleImage(GLenum format, GLint wi, GLint hi, GLenum typei, const void *datai, GLint wo, GLint ho, GLenum typeo,
                    void *datao);

#endif
