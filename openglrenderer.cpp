//
//#include "openglrenderer.h"
//#include <time.h>
//
//OpenGLRenderer::OpenGLRenderer()
//{
//    m_currentFrame = nullptr;
//    m_dbrightness = 0;
//    m_dcontrast = 0;
//    m_dfeather = 0;
//    m_dsaturation = 0;
//    m_scale = 1.0;
//    m_rotation = 0.0;
//
//    m_cropL = m_cropR = m_cropT = m_cropB = 0;
//    m_cropChanged = true;
//    m_bFrameUpdated = false;
//}
//
//OpenGLRenderer::~OpenGLRenderer()
//{
//}
//
//void OpenGLRenderer::initialize()
//{
//    const char *vsrc =
//        "#version 430 core\n \
//        layout(location = 0) in vec4 vertexIn; \
//    layout(location = 1) in vec2 textureIn; \
//    out vec2 textureOut;  \
//    uniform float scale;\
//    uniform float rotation;\
//    uniform mat4  cropMat;\
//    void main(void)           \
//    {                         \
//        mat4 scaleMat = mat4(scale,0,0,0,\
//                            0,scale,0,0,\
//                            0,0,1,0,\
//                            0,0,0,1);\
//\
//        mat4  rotMat = mat4(cos(rotation),sin(rotation),0,0,\
//            -sin(rotation),cos(rotation),0,0,\
//            0,0,1,0,\
//            0,0,0,1);\
//\
//        gl_Position =  rotMat * scaleMat * cropMat * vertexIn; \
//        textureOut = textureIn; \
//    }";
//
//    const char * fsrc =
//        "#version 430 core\n \
//        in vec2 textureOut;\
//    uniform sampler2D tex_y;\
//    uniform sampler2D tex_u;\
//    uniform sampler2D tex_v;\
//    uniform float feather;\
//    uniform float contrast;\
//    uniform float brightness;\
//    uniform float saturation;\
//    \
//    void contrastFilter(inout vec3 rgb)\
//    {\
//        rgb.r = clamp((rgb.r-0.5) * contrast + rgb.r, 0.0, 1.0); \
//        rgb.g = clamp((rgb.g-0.5) * contrast + rgb.g, 0.0, 1.0); \
//        rgb.b = clamp((rgb.b-0.5) * contrast + rgb.b, 0.0, 1.0); \
//    }\
//    void brightnessFilter(inout vec3 rgb)\
//    {\
//        rgb.r = clamp((1 + sin(brightness * 3.1415926 / 2))* rgb.r, 0.0, 1.0); \
//        rgb.g = clamp((1 + sin(brightness * 3.1415926 / 2))* rgb.g, 0.0, 1.0); \
//        rgb.b = clamp((1 + sin(brightness * 3.1415926 / 2))* rgb.b, 0.0, 1.0); \
//    }\
//    void featherFilter(inout vec3 rgb)\
//    {\
//        vec2 center = vec2(0.5, 0.5); \
//        float diff = dot(center, center) / feather; \
//        float dx = center.x - textureOut.x; \
//        float dy = center.y - textureOut.y; \
//        float distance_2 = pow(dx, 2.) + pow(dy, 2.); \
//        float v = distance_2 / diff; \
//        rgb.r = clamp(rgb.r + v, 0., 1.); \
//        rgb.g = clamp(rgb.g + v, 0., 1.);\
//        rgb.b = clamp(rgb.b + v, 0., 1.); \
//    }\
//    void saturationFilter(inout vec3 rgb)\
//    {\
//        float rgbMax = max(rgb.r, max(rgb.g, rgb.b)); \
//        float rgbMin = min(rgb.r, min(rgb.g, rgb.b)); \
//        float delta = (rgbMax - rgbMin); \
//        if (delta > 0)\
//        {\
//            float value = (rgbMax + rgbMin); \
//            float L = value / 2; \
//            float S; \
//            if (L < 0.5) S = delta / value; \
//            else S = delta / (2 - value); \
//            float alpha; \
//            if (saturation >= 0)\
//            {\
//                if ((saturation + S) >= 1) alpha = S; \
//                else alpha = 1 - saturation; \
//                alpha = 1 / alpha - 1; \
//                rgb = rgb + (rgb - vec3(L, L, L)) * alpha; \
//            }\
//            else\
//            {\
//                alpha = saturation; \
//                rgb = vec3(L, L, L) + (rgb - vec3(L, L, L)) * (1 + alpha); \
//            }\
//        }\
//    }\
//    void main(void)\
//    {\
//        vec3 yuv;\
//        vec3 rgb;\
//        yuv.x = texture2D(tex_y, textureOut).r;\
//        yuv.y = texture2D(tex_u, textureOut).r - 0.5;\
//        yuv.z = texture2D(tex_v, textureOut).r - 0.5;\
//        rgb = mat3(1, 1, 1,\
//            0, -0.3455, 1.779,\
//            1.4075, -0.7169, 0) * yuv;\
//        contrastFilter(rgb);\
//        brightnessFilter(rgb); \
//        featherFilter(rgb);\
//        saturationFilter(rgb);\
//        gl_FragColor = vec4(rgb, 1.0); \
//    }";
//
//    initializeOpenGLFunctions();
//
//    static const GLfloat vertices[]
//    {
//
//        -1.0f,-1.0f,
//        -1.0f,+1.0f,
//        +1.0f,+1.0f,
//        +1.0f,-1.0f,
//
//        0.0f,1.0f,
//        0.0f,0.0f,
//        1.0f,0.0f,
//        1.0f,1.0f,
//    };
//
//    /*------------------use Qt OpenGL Function-----------------*/
//    vao.create();
//    vao.bind();
//
//    vbo.create();
//    vbo.bind();
//    vbo.allocate(vertices, sizeof(vertices));
//
//    program.addCacheableShaderFromSourceCode(QOpenGLShader::Vertex, vsrc);
//    program.addCacheableShaderFromSourceCode(QOpenGLShader::Fragment, fsrc);
//    program.link();
//    program.bind();
//    program.enableAttributeArray(0);
//    program.enableAttributeArray(1);
//    program.setAttributeBuffer(0, GL_FLOAT, 0, 2, 2 * sizeof(GLfloat));
//    program.setAttributeBuffer(1, GL_FLOAT, 8 * sizeof(GLfloat), 2, 2 * sizeof(GLfloat));
//
//    texUniformY = program.uniformLocation("tex_y");
//    texUniformU = program.uniformLocation("tex_u");
//    texUniformV = program.uniformLocation("tex_v");
//
//    glUniform1i(texUniformY, 1);
//    glUniform1i(texUniformU, 2);
//    glUniform1i(texUniformV, 3);
//
//    brightnessUniform = program.uniformLocation("brightness");
//    contrastUniform = program.uniformLocation("contrast");
//    saturationUniform = program.uniformLocation("saturation");
//    featherUniform = program.uniformLocation("feather");
//    scaleUniform = program.uniformLocation("scale");
//    rotationUniform = program.uniformLocation("rotation");
//    cropUniform = program.uniformLocation("cropMat");
//
//    GLuint textures[3];
//    glGenTextures(3, textures);
//    idY = textures[0];
//    idU = textures[1];
//    idV = textures[2];
//
//    glActiveTexture(GL_TEXTURE1);
//    glBindTexture(GL_TEXTURE_2D, idY);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//
//    glActiveTexture(GL_TEXTURE2);
//    glBindTexture(GL_TEXTURE_2D, idU);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//
//    glActiveTexture(GL_TEXTURE3);
//    glBindTexture(GL_TEXTURE_2D, idV);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//
//    glEnable(GL_DEPTH_TEST);
//    glClearColor(0.0, 0.0, 0.0, 1.0);
//
//}
//
//void OpenGLRenderer::update(Frame * ptr)
//{
//    std::lock_guard<std::mutex> lock(m_mutex);
//    if (nullptr == ptr)
//    {
//        return;
//    }
//    m_currentFrame = ptr;
//    ////最后一帧视频帧删除，当序列号为-2时，表示该视频已经播放结束，可以释放该视频的最后一帧m_currentFrame了
//    //if (-2 == ptr->getSerialNumber())
//    //{
//    //    if (nullptr != m_currentFrame)
//    //    {
//    //        delete m_currentFrame;
//    //        m_currentFrame = nullptr;
//    //    }
//    //    return;
//    //}
//    //else
//    //{
//    //    //正常的视频帧更新
//    //    m_currentFrame = ptr;
//    //}
//
//    m_bFrameUpdated = true;
//}
//
//void OpenGLRenderer::setBrightness(double value)
//{
//    m_dbrightness = value;
//}
//
//void OpenGLRenderer::setContrast(double value)
//{
//    m_dcontrast = value;
//}
//
//void OpenGLRenderer::setSaturation(double value)
//{
//    m_dsaturation = value;
//}
//
//void OpenGLRenderer::setFeather(double value)
//{
//    m_dfeather = value;
//}
//
//float OpenGLRenderer::getBrightness()
//{
//    return m_dbrightness;
//}
//
//float OpenGLRenderer::getContrast()
//{
//    return m_dcontrast;
//}
//
//float OpenGLRenderer::getSaturation()
//{
//    return m_dsaturation;
//}
//
//float OpenGLRenderer::getFeather()
//{
//    return m_dfeather;
//}
//
//int OpenGLRenderer::cropL()
//{
//    return m_cropL;
//}
//
//int OpenGLRenderer::cropR()
//{
//    return m_cropR;
//}
//
//int OpenGLRenderer::cropT()
//{
//    return m_cropT;
//}
//
//int OpenGLRenderer::cropB()
//{
//    return m_cropB;
//}
//
//void OpenGLRenderer::setCropL(int value)
//{
//    m_cropL = value;
//    cropChanged();
//}
//
//void OpenGLRenderer::setCropR(int value)
//{
//    m_cropR = value;
//    cropChanged();
//}
//
//void OpenGLRenderer::setCropT(int value)
//{
//    m_cropT = value;
//    cropChanged();
//}
//
//void OpenGLRenderer::setCropB(int value)
//{
//    m_cropB = value;
//    cropChanged();
//}
//
//void OpenGLRenderer::caculateCrop()
//{
//    int cropW = m_cropL + m_cropR;
//    int cropH = m_cropT + m_cropB;
//    //int w = m_currentFrame->core()->width;
//    //int h = m_currentFrame->core()->height;
//    int w = m_currentFrame->width;
//    int h = m_currentFrame->height;
//
//    float scaleX = w / (float)(w - cropW);
//    float scaleY = h / (float)(h - cropH);
//    float transX = (m_cropR - m_cropL) / float(w);
//    float transY = (m_cropT - m_cropB) / float(h);
//
//    cropMat.setToIdentity();
//    cropMat.scale(scaleX, scaleY, 1.0);
//    cropMat.translate(transX, transY);
//    m_cropChanged = false;
//}
//
//void OpenGLRenderer::cropChanged()
//{
//    m_cropChanged = true;
//}
//
//// by Wenhui Geng
//void OpenGLRenderer::scale()
//{
//    //  源的高宽比:h/w
//    if (!(m_currentFrame->width <= 0 || m_currentFrame->height <= 0))
//    {
//        float resRation = m_currentFrame->height * 1.0 / m_currentFrame->width;
//        GLfloat aspectRatio;
//        //float width = m_currentFrame->getScaleWidth();
//        //float height = m_currentFrame->getScaleHeight();
//        float width = m_currentFrame->width;
//        float height = m_currentFrame->height;
//
//        float viewHeight = 0.0f;
//        float viewRatio = 0.0f;
//        float  srcHeight = height;
//        if (width < height)
//        {
//            height = width * resRation;
//            viewRatio = height / srcHeight;
//        }
//        else
//        {
//            float  srcWidth = width;
//            width = height / resRation;
//            viewRatio = width / srcWidth;
//        }
//        if (viewRatio > 0.0f && viewRatio < 1.0f)
//        {
//            viewHeight = srcHeight / 2.0f - height / 2.0f;
//        }
//
//        glViewport(0, viewHeight, width, height);
//
//        glMatrixMode(GL_PROJECTION);
//        glLoadIdentity();
//        //建立裁剪区域
//        aspectRatio = (GLfloat)width / (GLfloat)height;
//        if (width < height)
//        {
//            glOrtho(-1.0, 1.0, -1.0 / aspectRatio, 1.0 / aspectRatio, 1.0, -1.0);
//        }
//        else
//        {
//            glOrtho(-1.0*aspectRatio, 1.0*aspectRatio, -1.0, 1.0, 1.0, -1.0);
//        }
//        glMatrixMode(GL_MODELVIEW);
//        glLoadIdentity();
//    }
//}
//
//void OpenGLRenderer::render()
//{
//    std::lock_guard<std::mutex> lock(m_mutex);
//
//    if (nullptr == m_currentFrame  )  return;
//
//    int w = m_currentFrame->width;
//    int h = m_currentFrame->height;
//
//    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//
//    glUniform1f(brightnessUniform, m_dbrightness);
//    glUniform1f(contrastUniform, m_dcontrast);
//    glUniform1f(saturationUniform, m_dsaturation);
//    glUniform1f(featherUniform, m_dfeather);
//
//    glUniform1f(scaleUniform, 1.0);
//    glUniform1f(rotationUniform, 0.0);
//
//    if (m_cropChanged)
//    {
//        caculateCrop();
//        glUniformMatrix4fv(cropUniform, 1, GL_FALSE, cropMat.constData());
//    }
//
//    //*************by Wenhui Geng***************
//    //if (m_currentFrame->getScaleMode() == enEqualRatio)
//    //{
//    //    scale();
//    //}
//    //*******************************************
//
//    if (m_bFrameUpdated)
//    {
//        glActiveTexture(GL_TEXTURE1);
//        glBindTexture(GL_TEXTURE_2D, idY);
//        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, w, h, 0, GL_RED, GL_UNSIGNED_BYTE, m_currentFrame->data);
//
//        glActiveTexture(GL_TEXTURE2);
//        glBindTexture(GL_TEXTURE_2D, idU);
//        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, w >> 1, h >> 1, 0, GL_RED, GL_UNSIGNED_BYTE, m_currentFrame->data);
//
//        glActiveTexture(GL_TEXTURE3);
//        glBindTexture(GL_TEXTURE_2D, idV);
//        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, w >> 1, h >> 1, 0, GL_RED, GL_UNSIGNED_BYTE, m_currentFrame->data);
//        m_bFrameUpdated = false;
//    }
//    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
//    glActiveTexture(GL_ACTIVE_TEXTURE);
//}
//
//#define VERTEXIN 0
//#define TEXTUREIN 1
//
//RgbRenderer::RgbRenderer() : m_currentFrame(nullptr)
//{
//}
//
//RgbRenderer::~RgbRenderer()
//{
//
//}
//
//void RgbRenderer::initialize()
//{
//    const char *vsrc =
//        "attribute vec4 vertexIn; \
//    attribute vec2 textureIn; \
//    varying vec2 textureOut;  \
//    void main(void)           \
//    {                         \
//        gl_Position = vertexIn; \
//        textureOut = textureIn; \
//    }";
//
//    const char *fsrc =
//        "varying vec2 textureOut; \
//    uniform sampler2D tex_rgb; \
//    void main(void) \
//    { \
//        vec4 rgba; \
//        rgba = texture2D(tex_rgb, textureOut); \
//        gl_FragColor = vec4(rgba); \
//    }";
//
//    initializeOpenGLFunctions();
//    glEnable(GL_DEPTH_TEST);
//
//    static const GLfloat vertices[]{
//
//        //顶点坐标
//        -1.0f,-1.0f,
//        -1.0f,+1.0f,
//        +1.0f,+1.0f,
//        +1.0f,-1.0f,
//        //纹理坐标
//        0.0f,1.0f,
//        0.0f,0.0f,
//        1.0f,0.0f,
//        1.0f,1.0f,
//    };
//
//    vbo.create();
//    vbo.bind();
//    vbo.allocate(vertices, sizeof(vertices));
//
//    program = new QOpenGLShaderProgram();
//    program->addCacheableShaderFromSourceCode(QOpenGLShader::Vertex, vsrc);
//    program->addCacheableShaderFromSourceCode(QOpenGLShader::Fragment, fsrc);
//    program->link();
//
//    program->bindAttributeLocation("vertexIn", VERTEXIN);
//    program->bindAttributeLocation("textureIn", TEXTUREIN);
//    program->bind();
//
//    program->enableAttributeArray(VERTEXIN);
//    program->enableAttributeArray(TEXTUREIN);
//
//    program->setAttributeBuffer(VERTEXIN, GL_FLOAT, 0, 2, 2 * sizeof(GLfloat));
//    program->setAttributeBuffer(TEXTUREIN, GL_FLOAT, 8 * sizeof(GLfloat), 2, 2 * sizeof(GLfloat));
//
//    textureUniformRgb = program->uniformLocation("tex_rgb");
//
//    textureRgb = new QOpenGLTexture(QOpenGLTexture::Target2D);
//
//    textureRgb->create();
//
//    idRgb = textureRgb->textureId();
//
//    glClearColor(0.0, 0.0, 0.0, 0.0);
//}
//
//void RgbRenderer::update(Frame * ptr)
//{
//    std::lock_guard<std::mutex> lock(m_mutex);
//    if (nullptr == ptr)
//    {
//        return;
//    }
//    //最后一帧视频帧删除，当序列号为-2时，表示该视频已经播放结束，可以释放该视频的最后一帧m_currentFrame了
//    //if (-2 == ptr->getSerialNumber())
//    //{
//    //    if (nullptr != m_currentFrame)
//    //    {
//    //        delete m_currentFrame;
//    //        m_currentFrame = nullptr;
//    //    }
//    //    return;
//    //}
//    else
//    {
//        //正常的视频帧更新
//        m_currentFrame = ptr;
//    }
//    m_bFrameUpdated = true;
//}
//
//void RgbRenderer::render()
//{
//    std::lock_guard<std::mutex> lock(m_mutex);
//
//    if (nullptr == m_currentFrame /*|| nullptr == m_currentFrame->core()*/)  return;
//    int w = m_currentFrame->width;
//    int h = m_currentFrame->height;
//
//    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//    glActiveTexture(GL_TEXTURE0);
//    glBindTexture(GL_TEXTURE_2D, idRgb);
//
//    glEnable(GL_BLEND);  //开启混合模式
//    glBlendFunc(GL_SRC_ALPHA, GL_ZERO);
//
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//
//    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_SHORT, m_currentFrame->data);
//
//    glUniform1i(textureUniformRgb, 0);
//
//    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
//}
//
//
//void RgbRenderer::setBrightness(double value)
//{
//    m_dbrightness = value;
//}
//
//void RgbRenderer::setContrast(double value)
//{
//    m_dcontrast = value;
//}
//
//void RgbRenderer::setSaturation(double value)
//{
//    m_dsaturation = value;
//}
//
//void RgbRenderer::setFeather(double value)
//{
//    m_dfeather = value;
//}
//
//float RgbRenderer::getBrightness()
//{
//    return m_dbrightness;
//}
//
//float RgbRenderer::getContrast()
//{
//    return m_dcontrast;
//}
//
//float RgbRenderer::getSaturation()
//{
//    return m_dsaturation;
//}
//
//float RgbRenderer::getFeather()
//{
//    return m_dfeather;
//}
//
//int RgbRenderer::cropL()
//{
//    return m_cropL;
//}
//
//int RgbRenderer::cropR()
//{
//    return m_cropR;
//}
//
//int RgbRenderer::cropT()
//{
//    return m_cropT;
//}
//
//int RgbRenderer::cropB()
//{
//    return m_cropB;
//}
//
//void RgbRenderer::setCropL(int value)
//{
//    m_cropL = value;
//}
//
//void RgbRenderer::setCropR(int value)
//{
//    m_cropR = value;
//}
//
//void RgbRenderer::setCropT(int value)
//{
//    m_cropT = value;
//}
//
//void RgbRenderer::setCropB(int value)
//{
//    m_cropB = value;
//}
//
//
//
