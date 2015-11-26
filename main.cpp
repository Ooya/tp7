/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the documentation of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "openglwindow.h"

#include <QtGui/QGuiApplication>
#include <QtGui/QMatrix4x4>
#include <QtGui/QOpenGLShaderProgram>
#include <QtGui/QScreen>

#include <QtCore/qmath.h>

class TriangleWindow : public OpenGLWindow
{
public:
    TriangleWindow();

    static const int tailleX = 240;
    static const int tailleY = 240;

    QImage relief = QImage(":/heightmap-1.png");

    void initialize() Q_DECL_OVERRIDE;
    void render() Q_DECL_OVERRIDE;

private:
    GLuint loadShader(GLenum type, const char *source);

    GLuint m_posAttr;
    GLuint m_colAttr;
    GLuint m_matrixUniform;

    QOpenGLShaderProgram *m_program;
    int m_frame;
    GLfloat getZ(GLfloat i, GLfloat j);
    GLfloat *getColors(GLint x, GLint y);
    GLfloat *getPoints(GLint x, GLint y);
    //GLfloat getCube(GLint taille);
};

TriangleWindow::TriangleWindow()
    : m_program(0)
    , m_frame(0)
{
}
int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);

    QSurfaceFormat format;
    format.setSamples(16);

    TriangleWindow window;
    window.setFormat(format);
    window.resize(800, 600);
    window.show();

    window.setAnimating(true);

    return app.exec();
}

static const char *vertexShaderSource =
    "attribute highp vec4 posAttr;\n"
    "attribute lowp vec4 colAttr;\n"
    "varying lowp vec4 col;\n"
    "uniform highp mat4 matrix;\n"
    "void main() {\n"
    "   col = colAttr;\n"
    "   gl_Position = matrix * posAttr;\n"
    "}\n";

static const char *fragmentShaderSource =
    "varying lowp vec4 col;\n"
    "void main() {\n"
    "   gl_FragColor = col;\n"
    "}\n";
//! [3]

//! [4]
GLuint TriangleWindow::loadShader(GLenum type, const char *source){
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, 0);
    glCompileShader(shader);
    return shader;
}

void TriangleWindow::initialize(){
    m_program = new QOpenGLShaderProgram(this);
    m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource);
    m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource);
    m_program->link();
    m_posAttr = m_program->attributeLocation("posAttr");
    m_colAttr = m_program->attributeLocation("colAttr");
    m_matrixUniform = m_program->uniformLocation("matrix");
}

GLfloat TriangleWindow::getZ(GLfloat i, GLfloat j){
    return qGray(this->relief.pixel((this->tailleX * (i+0.5f)), (this->tailleY * (j+0.5f)))) * 0.0008f;
}

GLfloat *TriangleWindow::getColors(GLint x, GLint y){
    GLint nb = x * y * 3 * 2 + x * 3 + 3;
    GLfloat *couleurs= new GLfloat[nb];
    for(GLint i = 0;i<(nb);i+=3){
        couleurs[i] = 0.0f;
        couleurs[i+1] = 1.0f;
        couleurs[i+2] = 0.0f;
    }

    return couleurs;
}

GLfloat *TriangleWindow::getPoints(GLint x, GLint y){
    GLint nb = x * y * 3 * 2 + x * 3 + 3;
    GLfloat *points = new GLfloat[nb];
    GLfloat stepX = 1.0f / (x);
    GLfloat stepY = 1.0f / (y);
    GLint cpt = 0;
    GLint k = 1;
    GLfloat posX = -0.5f;
    GLfloat posY = -0.5f;

    for (GLint i = 0; i < x; ++i) {
        for (GLint j = 0; j < y; ++j) {
            points[cpt++] = posX;
            points[cpt++] = getZ(posX, posY);
            points[cpt++] = posY;
            points[cpt++] = posX + stepX;
            points[cpt++] = getZ(posX + stepX, posY);
            points[cpt++] = posY;
            posY += stepY * k;
        }
        points[cpt++] = posX;
        points[cpt++] = getZ(posX, posY);
        points[cpt++] = posY;
        k *= -1;
        posX += stepX;
    }
    points[cpt++] = posX;
    points[cpt++] = getZ(posX, posY);
    points[cpt++] = posY;

    return points;
}

//GLfloat TriangleWindow::getCube(GLint taille){
//    GLfloat x = taille / 1200.0f;
//    //GLfloat *points = new GLfloat[24];

//    GLfloat points[] = {
//        -x,-x,x, x,-x,x, -x,x,x, x,x,x,
//        x,-x,x, x,-x,-x, x,x,x, x,x,-x,
//        x,-x,-x, -x,-x,-x, x,x,-x, -x,x,-x,
//        -x,-x,-x, -x,-x,x, -x,x,-x, -x,x,x,
//        -x,-x,-x, x,-x,-x, -x,-x,x, x,-x,x,
//        -x,x,x, x,x,x, -x,x,-x, x,x,-x
//    };

//    return *points;
//}


void TriangleWindow::render()
{
    const qreal retinaScale = devicePixelRatio();
    glViewport(0, 0, width() * retinaScale, height() * retinaScale);

    glClear(GL_COLOR_BUFFER_BIT);

    m_program->bind();

    QMatrix4x4 matrix;
    matrix.perspective(60.0f, 16.0f/9.0f, 0.1f, 100.0f);
    matrix.translate(0, -0.13f, -0.6f);
    matrix.rotate(100.0f * m_frame / screen()->refreshRate(), 0, 1, 0);

    m_program->setUniformValue(m_matrixUniform, matrix);

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);


    GLfloat cube[] = {
        -0.2,-0.2,0.2, 0.2,-0.2,0.2, -0.2,0.2,0.2, 0.2,0.2,0.2,
        0.2,-0.2,0.2, 0.2,-0.2,-0.2, 0.2,0.2,0.2, 0.2,0.2,-0.2,
        0.2,-0.2,-0.2, -0.2,-0.2,-0.2, 0.2,0.2,-0.2, -0.2,0.2,-0.2,
        -0.2,-0.2,-0.2, -0.2,-0.2,0.2, -0.2,0.2,-0.2, -0.2,0.2,0.2,
        -0.2,-0.2,-0.2, 0.2,-0.2,-0.2, -0.2,-0.2,0.2, 0.2,-0.2,0.2,
        -0.2,0.2,0.2, 0.2,0.2,0.2, -0.2,0.2,-0.2, 0.2,0.2,-0.2
    };

//    GLfloat colors[] = {
//        0.0f, 1.0f, 0.0f
//    };


    glVertexAttribPointer(m_posAttr, 3, GL_FLOAT, GL_FALSE, 0, getPoints(tailleX, tailleY));
    glVertexAttribPointer(m_colAttr, 3, GL_FLOAT, GL_FALSE, 0, getColors(tailleX, tailleY));
    //glVertexAttribPointer(m_posAttr, 3, GL_FLOAT, GL_FALSE, 0, cube);
    //glVertexAttribPointer(m_colAttr, 3, GL_FLOAT, GL_FALSE, 0, getColors(6, 4));

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, tailleX*tailleY*2+tailleX+1);
    //glDrawArrays(GL_TRIANGLE_STRIP, 0, tailleX*tailleY*2+tailleX+1);

    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(0);

    m_program->release();

    ++m_frame;
}
//! [5]
