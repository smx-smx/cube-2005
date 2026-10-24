// gles1compat.cpp: desktop OpenGL 1.x / GLU subset on OpenGL ES 1.1, see gles1compat.h

#define GLES1COMPAT_IMPL
#include "gles1compat.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

namespace {

struct Vertex { GLfloat x, y, z, s, t, r, g, b, a; };

struct Primitive
{
    GLenum mode;                            // GLES mode
    std::vector<Vertex> verts;
};

struct ClientArray
{
    bool enabled;
    GLint size;
    GLenum type;
    GLsizei stride;
    const GLvoid *ptr;
};

// immediate mode
GLenum imMode;
bool imActive;
std::vector<Vertex> imVerts;
GLfloat curColor[4] = {1, 1, 1, 1};
GLfloat curTex[2] = {0, 0};
bool lineMode;

// display lists
std::vector<std::vector<Primitive> > lists;
GLuint compiling;                           // list being compiled, 0 if none

// the application's client arrays, restored after immediate mode draws
ClientArray appVertex, appColor, appTexCoord;

ClientArray *appArray(GLenum array)
{
    switch(array)
    {
        case GL_VERTEX_ARRAY: return &appVertex;
        case GL_COLOR_ARRAY: return &appColor;
        case GL_TEXTURE_COORD_ARRAY: return &appTexCoord;
        default: return NULL;
    }
}

void restoreArray(GLenum array, const ClientArray &a)
{
    if(a.enabled) glEnableClientState(array); else glDisableClientState(array);
    if(!a.ptr) return;
    switch(array)
    {
        case GL_VERTEX_ARRAY: glVertexPointer(a.size, a.type, a.stride, a.ptr); break;
        case GL_COLOR_ARRAY: glColorPointer(a.size, a.type, a.stride, a.ptr); break;
        case GL_TEXTURE_COORD_ARRAY: glTexCoordPointer(a.size, a.type, a.stride, a.ptr); break;
    }
}

void draw(const Primitive &p)
{
    if(p.verts.empty()) return;
    const Vertex *v = &p.verts[0];
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glVertexPointer(3, GL_FLOAT, sizeof(Vertex), &v->x);
    glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), &v->s);
    glColorPointer(4, GL_FLOAT, sizeof(Vertex), &v->r);
    glDrawArrays(p.mode, 0, (GLsizei)p.verts.size());
    restoreArray(GL_VERTEX_ARRAY, appVertex);
    restoreArray(GL_COLOR_ARRAY, appColor);
    restoreArray(GL_TEXTURE_COORD_ARRAY, appTexCoord);
    // the current color is undefined after drawing with a color array
    glColor4f(curColor[0], curColor[1], curColor[2], curColor[3]);
}

// convert desktop primitives to GLES ones, polygons become line loops in GL_LINE mode
void submit(GLenum mode, const std::vector<Vertex> &verts)
{
    std::vector<Primitive> prims;
    size_t n = verts.size();
    switch(mode)
    {
        case GL_QUADS:
            if(lineMode)
            {
                for(size_t i = 0; i+3 < n; i += 4)
                {
                    Primitive p = { GL_LINE_LOOP, std::vector<Vertex>(verts.begin()+i, verts.begin()+i+4) };
                    prims.push_back(p);
                }
            }
            else
            {
                Primitive p = { GL_TRIANGLES, std::vector<Vertex>() };
                for(size_t i = 0; i+3 < n; i += 4)
                {
                    static const int order[6] = {0, 1, 2, 0, 2, 3};
                    for(int k = 0; k < 6; k++) p.verts.push_back(verts[i+order[k]]);
                }
                prims.push_back(p);
            }
            break;
        case GL_POLYGON:
        {
            Primitive p = { (GLenum)(lineMode ? GL_LINE_LOOP : GL_TRIANGLE_FAN), verts };
            prims.push_back(p);
            break;
        }
        case GL_QUAD_STRIP:
        {
            // same vertex order as a triangle strip
            Primitive p = { GL_TRIANGLE_STRIP, verts };
            prims.push_back(p);
            break;
        }
        default:
        {
            Primitive p = { mode, verts };
            prims.push_back(p);
            break;
        }
    }
    for(size_t i = 0; i < prims.size(); i++)
    {
        if(compiling) lists[compiling].push_back(prims[i]);
        else draw(prims[i]);
    }
}

}

void cgl_Begin(GLenum mode)
{
    imMode = mode;
    imActive = true;
    imVerts.clear();
}

void cgl_End()
{
    imActive = false;
    submit(imMode, imVerts);
}

void cgl_Vertex3f(GLfloat x, GLfloat y, GLfloat z)
{
    Vertex v = { x, y, z, curTex[0], curTex[1], curColor[0], curColor[1], curColor[2], curColor[3] };
    imVerts.push_back(v);
}

void cgl_TexCoord2f(GLfloat s, GLfloat t)
{
    curTex[0] = s;
    curTex[1] = t;
}

void cgl_Color4f(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
    curColor[0] = r; curColor[1] = g; curColor[2] = b; curColor[3] = a;
    if(!imActive) glColor4f(r, g, b, a);
}

GLuint cgl_GenLists(GLsizei range)
{
    if(lists.empty()) lists.resize(1);      // list 0 is invalid
    GLuint first = (GLuint)lists.size();
    lists.resize(lists.size()+range);
    return first;
}

void cgl_NewList(GLuint list, GLenum mode)
{
    // cube uses fixed list numbers without glGenLists, and only GL_COMPILE
    (void)mode;
    if(list >= lists.size()) lists.resize(list+1);
    lists[list].clear();
    compiling = list;
}

void cgl_EndList()
{
    compiling = 0;
}

void cgl_CallList(GLuint list)
{
    if(list >= lists.size()) return;
    for(size_t i = 0; i < lists[list].size(); i++) draw(lists[list][i]);
}

void cgl_PolygonMode(GLenum face, GLenum mode)
{
    (void)face;
    lineMode = mode == GL_LINE;
}

void cgl_Enable(GLenum cap)
{
    if(cap == GL_POLYGON_OFFSET_LINE) return;   // no line offset in GLES
    glEnable(cap);
}

void cgl_Disable(GLenum cap)
{
    if(cap == GL_POLYGON_OFFSET_LINE) return;
    glDisable(cap);
}

void cgl_VertexPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *ptr)
{
    ClientArray a = { appVertex.enabled, size, type, stride, ptr };
    appVertex = a;
    glVertexPointer(size, type, stride, ptr);
}

void cgl_ColorPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *ptr)
{
    ClientArray a = { appColor.enabled, size, type, stride, ptr };
    appColor = a;
    glColorPointer(size, type, stride, ptr);
}

void cgl_TexCoordPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *ptr)
{
    ClientArray a = { appTexCoord.enabled, size, type, stride, ptr };
    appTexCoord = a;
    glTexCoordPointer(size, type, stride, ptr);
}

void cgl_EnableClientState(GLenum array)
{
    if(ClientArray *a = appArray(array)) a->enabled = true;
    glEnableClientState(array);
}

void cgl_DisableClientState(GLenum array)
{
    if(ClientArray *a = appArray(array)) a->enabled = false;
    glDisableClientState(array);
}

void cgl_ReadPixels(GLint x, GLint y, GLsizei w, GLsizei h, GLenum format, GLenum type, GLvoid *pixels)
{
    if(format == GL_DEPTH_COMPONENT)
    {
        // GLES can't read the depth buffer: report the far plane (only used by the edit cursor)
        for(GLsizei i = 0; i < w*h; i++) ((GLfloat *)pixels)[i] = 1.0f;
        return;
    }
    if(format == GL_RGB && type == GL_UNSIGNED_BYTE)
    {
        GLubyte *rgba = (GLubyte *)malloc(w*h*4);
        if(!rgba) return;
        glReadPixels(x, y, w, h, GL_RGBA, GL_UNSIGNED_BYTE, rgba);
        for(GLsizei i = 0; i < w*h; i++) memcpy((GLubyte *)pixels + i*3, rgba + i*4, 3);
        free(rgba);
        return;
    }
    glReadPixels(x, y, w, h, format, type, pixels);
}

void cgl_LoadMatrixd(const GLdouble *m)
{
    GLfloat f[16];
    for(int i = 0; i < 16; i++) f[i] = (GLfloat)m[i];
    glLoadMatrixf(f);
}

void cgl_GetDoublev(GLenum pname, GLdouble *params)
{
    GLfloat f[16];
    glGetFloatv(pname, f);
    int n = (pname == GL_MODELVIEW_MATRIX || pname == GL_PROJECTION_MATRIX || pname == GL_TEXTURE_MATRIX) ? 16 : 1;
    for(int i = 0; i < n; i++) params[i] = f[i];
}

// GLU

void gluPerspective(GLdouble fovy, GLdouble aspect, GLdouble znear, GLdouble zfar)
{
    GLdouble ymax = znear*tan(fovy*M_PI/360.0), xmax = ymax*aspect;
    glFrustumf((GLfloat)-xmax, (GLfloat)xmax, (GLfloat)-ymax, (GLfloat)ymax, (GLfloat)znear, (GLfloat)zfar);
}

namespace {

void multMatrixVec(const GLdouble m[16], const GLdouble in[4], GLdouble out[4])
{
    for(int i = 0; i < 4; i++)
        out[i] = in[0]*m[0*4+i] + in[1]*m[1*4+i] + in[2]*m[2*4+i] + in[3]*m[3*4+i];
}

void multMatrices(const GLdouble a[16], const GLdouble b[16], GLdouble r[16])
{
    for(int i = 0; i < 4; i++) for(int j = 0; j < 4; j++)
        r[i*4+j] = a[i*4+0]*b[0*4+j] + a[i*4+1]*b[1*4+j] + a[i*4+2]*b[2*4+j] + a[i*4+3]*b[3*4+j];
}

// Gauss-Jordan inversion with partial pivoting
bool invertMatrix(const GLdouble m[16], GLdouble inv[16])
{
    GLdouble t[4][8];
    for(int i = 0; i < 4; i++) for(int j = 0; j < 4; j++) { t[i][j] = m[j*4+i]; t[i][j+4] = i == j; }
    for(int c = 0; c < 4; c++)
    {
        int p = c;
        for(int r = c+1; r < 4; r++) if(fabs(t[r][c]) > fabs(t[p][c])) p = r;
        if(t[p][c] == 0) return false;
        if(p != c) for(int k = 0; k < 8; k++) { GLdouble x = t[c][k]; t[c][k] = t[p][k]; t[p][k] = x; }
        GLdouble d = t[c][c];
        for(int k = 0; k < 8; k++) t[c][k] /= d;
        for(int r = 0; r < 4; r++) if(r != c)
        {
            GLdouble f = t[r][c];
            for(int k = 0; k < 8; k++) t[r][k] -= f*t[c][k];
        }
    }
    for(int i = 0; i < 4; i++) for(int j = 0; j < 4; j++) inv[j*4+i] = t[i][j+4];
    return true;
}

}

GLint gluUnProject(GLdouble winx, GLdouble winy, GLdouble winz, const GLdouble model[16], const GLdouble proj[16],
                   const GLint viewport[4], GLdouble *objx, GLdouble *objy, GLdouble *objz)
{
    GLdouble pm[16], inv[16], in[4], out[4];
    multMatrices(model, proj, pm);
    if(!invertMatrix(pm, inv)) return GL_FALSE;
    in[0] = (winx-viewport[0])/viewport[2]*2-1;
    in[1] = (winy-viewport[1])/viewport[3]*2-1;
    in[2] = winz*2-1;
    in[3] = 1;
    multMatrixVec(inv, in, out);
    if(out[3] == 0) return GL_FALSE;
    *objx = out[0]/out[3];
    *objy = out[1]/out[3];
    *objz = out[2]/out[3];
    return GL_TRUE;
}

GLUquadric *gluNewQuadric()
{
    return (GLUquadric *)calloc(1, sizeof(GLUquadric));
}

void gluDeleteQuadric(GLUquadric *q) { free(q); }
void gluQuadricDrawStyle(GLUquadric *q, GLenum style) { (void)q; (void)style; }
void gluQuadricOrientation(GLUquadric *q, GLenum orientation) { q->inside = orientation == GLU_INSIDE; }
void gluQuadricTexture(GLUquadric *q, GLboolean textured) { q->textured = textured; }

// filled sphere as quad strips, like Mesa's GLU (the inside orientation mirrors z)
void gluSphere(GLUquadric *q, GLdouble radius, GLint slices, GLint stacks)
{
    GLdouble drho = M_PI/stacks, dtheta = 2*M_PI/slices, ds = 1.0/slices, dt = 1.0/stacks, t = 1.0;
    GLdouble nsign = q->inside ? -1.0 : 1.0;
    for(int i = 0; i < stacks; i++)
    {
        GLdouble rho = i*drho, s = 0.0;
        cgl_Begin(GL_QUAD_STRIP);
        for(int j = 0; j <= slices; j++)
        {
            GLdouble theta = j == slices ? 0.0 : j*dtheta;
            if(q->textured) cgl_TexCoord2f((GLfloat)s, (GLfloat)t);
            cgl_Vertex3f((GLfloat)(-sin(theta)*sin(rho)*radius), (GLfloat)(cos(theta)*sin(rho)*radius),
                         (GLfloat)(nsign*cos(rho)*radius));
            if(q->textured) cgl_TexCoord2f((GLfloat)s, (GLfloat)(t-dt));
            cgl_Vertex3f((GLfloat)(-sin(theta)*sin(rho+drho)*radius), (GLfloat)(cos(theta)*sin(rho+drho)*radius),
                         (GLfloat)(nsign*cos(rho+drho)*radius));
            s += ds;
        }
        cgl_End();
        t -= dt;
    }
}

namespace {

int componentsOf(GLenum format)
{
    switch(format)
    {
        case GL_RGBA: return 4;
        case GL_RGB: return 3;
        case GL_LUMINANCE_ALPHA: return 2;
        default: return 1;
    }
}

GLint floorPow2(GLint x)
{
    GLint p = 1;
    while(p*2 <= x) p *= 2;
    return p;
}

}

// nearest neighbour, GL_UNSIGNED_BYTE only
GLint gluScaleImage(GLenum format, GLint wi, GLint hi, GLenum typei, const void *datai, GLint wo, GLint ho, GLenum typeo,
                    void *datao)
{
    if(typei != GL_UNSIGNED_BYTE || typeo != GL_UNSIGNED_BYTE) return GL_INVALID_ENUM;
    int c = componentsOf(format);
    for(GLint y = 0; y < ho; y++) for(GLint x = 0; x < wo; x++)
        memcpy((GLubyte *)datao + (y*wo+x)*c, (const GLubyte *)datai + ((y*hi/ho)*wi + x*wi/wo)*c, c);
    return 0;
}

// GLES 1.1 needs power of two sizes, scale down like GLU does and let the GPU build the mipmaps
GLint gluBuild2DMipmaps(GLenum target, GLint components, GLint w, GLint h, GLenum format, GLenum type, const void *data)
{
    (void)components;
    GLint w2 = floorPow2(w), h2 = floorPow2(h);
    void *scaled = NULL;
    if(w2 != w || h2 != h)
    {
        scaled = malloc(w2*h2*componentsOf(format));
        if(!scaled) return GL_OUT_OF_MEMORY;
        gluScaleImage(format, w, h, type, data, w2, h2, type, scaled);
        data = scaled;
    }
    glTexParameteri(target, GL_GENERATE_MIPMAP, GL_TRUE);
    glTexImage2D(target, 0, format, w2, h2, 0, format, type, data);
    free(scaled);
    return 0;
}
