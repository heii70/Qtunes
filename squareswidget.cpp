#include "squareswidget.h"
#include <glu.h>
#include <QtWidgets>
#include <QGLWidget>

/* SquaresWidget constructor that sets the default values of several
 * variables and initializes the QLists used in this widget. It also
 * initializes a QTimer and connects it such that it will call the slot
 * function update() every 16ms. update() will call updateGL(), which will
 * call paintGL().
 * */
SquaresWidget::SquaresWidget(){
	m_width = 3;
	m_height = 1;
	m_scale = 250;
    setMinimumSize(m_width*m_scale,m_height*m_scale);
    resize(m_width*m_scale,m_height*m_scale);
	setMaximumSize(m_width*m_scale,m_height*m_scale);
    setSizeIncrement(m_width,m_height);
	m_translate = 0;
	m_translateBuffer = 0;
	m_recordsLoaded = false;
    m_fromMP3 = false;
    m_numAlbums = 0;
	m_directory = ".";
    m_centerRegion = 0;
    m_shift = 2.000;
    m_albumDepth = 1.0;
    m_initialCall = true;
    m_albumsShown = 0;
    m_doubleClicked = false;
    m_mainAlbum = 0;
    m_albumList = new QList<QString>;
    m_glTexID = new QList<GLuint>;
	m_timer = new QTimer(this);
	m_timer->start(16);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(update()));
}

/* SquaresWidget deconstructor*/
SquaresWidget::~SquaresWidget(){
}

/* shiftLeft() is a function that increments m_translate by m_shift. This
 * function is called when the mousePressEvent function determines that there
 * is a mouse click on the left side of the widget (this wil be similar to
 * clicking on an album to the left of the center widget). If the coverflow is
 * too far to the left, m_translate will not be incremented.
 */
void SquaresWidget::shiftLeft(){
    if(m_numAlbums == 0) return;
    if((int)(-(m_translate-0.01)/m_shift+m_numAlbums/2) <= 0.05) return;
	m_translate += m_shift;
}

/* shiftRight() is a function that decrements m_translate by m_shift. This
 * function is called when the mousePressEvent function determines that there
 * is a mouse click on the right side of the widget (this wil be similar to
 * clicking on an album to the right of the center widget). If the coverflow is
 * too far to the right, m_translate will not be decremented.
 */
void SquaresWidget::shiftRight(){
    if(m_numAlbums == 0) return;
    if((int)(-(m_translate-0.01)/m_shift+m_numAlbums/2) >= m_numAlbums-1) return;
    m_translate -= m_shift;
}

/* s_MP3Art is a slot function that is connected to a signal from MainWindow that
 * is emitted when the s_load function is called. It takes in a QList of
 * QImages that is passed to it by MainWindow and appends to a QList of QImages,
 * m_glTexID. m_glTexID contains GLuints that can be used to create the coverflow
 * from. This function also takes in a QList of QStrings that is used to save the
 * name of the QImages that have been passed to this function.
 * */
void SquaresWidget::s_MP3Art(QList<QImage> *artlist, QList<QString> *albumlist){
    if(artlist->size() == 0 || albumlist->size() == 0) return;
    const QList<QString> *temp_list(albumlist);
    m_albumList->append(*temp_list);
    m_fromMP3 = true;
    m_recordsLoaded = true;
    m_numAlbums += artlist->size();

    /* This for loop essentially converts the QImages passed to the function into
     * GLuints that OpenGL can used to display these images in the coverflow.
    */
	glEnable(GL_TEXTURE_2D);
    for(int i=0; i < artlist->size(); i++) {
		QString temp_string = QString("record %1").arg(i+1);
        GLuint tempGLu;
        glGenTextures(1, &tempGLu);
        glBindTexture(GL_TEXTURE_2D, tempGLu);
	    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 
		QImage image_gl = QGLWidget::convertToGLFormat(artlist->at(i));
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_gl.width(),
            image_gl.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, image_gl.bits());
        m_glTexID->append(tempGLu);
	}
	glDisable(GL_TEXTURE_2D);

    // Determines the amount of albums actually rendered by this widget.
    if(m_numAlbums < 8) m_albumsShown = m_numAlbums;
    else m_albumsShown = 7;
}

/* mousePressEvent is a function that will save the x-coordinate and y-coordinate
 * whenever the mouse is clicked on the widget. If the mouse click is on the album
 * that is:
 *  2 albums to the left, then shiftLeft is called twice
 *  1 album to the left, then shiftLeft is called once
 *  1 album to the right, then shiftRight is called once
 *  2 albums to the right, then shiftRight is called twice
 */
void SquaresWidget::mousePressEvent(QMouseEvent *event){
    m_xpos = event->x();
    m_ypos = event->y();
    if(m_xpos < 0.175*width()){
        shiftLeft();
        shiftLeft();
    }
    else if(m_xpos <= 0.34*width()) shiftLeft();
    else if(m_xpos >= 0.66*width() && m_xpos < 0.825*width()) shiftRight();
    else if(m_xpos >= 0.825*width() && m_xpos <= width()){
        shiftRight();
        shiftRight();
    }
}

/* mouseDoubleClickEvent is a function that sets boolean m_doubleClicked to true
 * if this event takes place horizontally close to the middle of the widget.
 * This corresponds to doubleclicking the center album art.
 */
void SquaresWidget::mouseDoubleClickEvent(QMouseEvent *event){
    if(event->x() > 0.34*width() && event->x() < 0.66*width()) m_doubleClicked = true;
}

/* defaultImage loads the default image that will be used when there has not been
 * any music loaded yet into QTunes.
 */
void SquaresWidget::defaultImage(){
    QImage def_image, resized_image;
    def_image.load(":/Resources/noload.jpg");
    resized_image = def_image.scaled(250,250,Qt::KeepAspectRatio);

    // Converts the default image into a GLuint that can be used in OpenGL.
    glEnable(GL_TEXTURE_2D);
    glGenTextures(1,&m_defTexID);
    glBindTexture(GL_TEXTURE_2D, m_defTexID);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    QImage def_gl = QGLWidget::convertToGLFormat(resized_image);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, def_gl.width(), def_gl.height(), 0,
        GL_RGBA, GL_UNSIGNED_BYTE, def_gl.bits());
    glDisable(GL_TEXTURE_2D);
}

/* initializeGL is a function that sets up the widget to be used. It clears the
 * widget of any previous colors, enables the widget to display things in 3D
 * correctly, and enables certain shapes to have smooth lines. initializeGL also
 * calls defaultImage to load the default image to be used as an OpenGL texture.
 * */
void SquaresWidget::initializeGL(){
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LINE_SMOOTH);
    defaultImage();
}

/* resizeGL is a function that sets up the perspective view for the widget.
 * This function also sets up the camera view, and loads the identity
 * matrix into the widget.
 */
void SquaresWidget::resizeGL(int w, int h){
    glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60, (float) w/h, 1, 1000);
    gluLookAt(0,-0.05, 2.95, //2.95
                0,-0.05,1,
                0,1,0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

/* paintGL is a function that is updated every 16ms in order to properly render
 * the coverflow. This function creates album coverflow from mp3 files and allows
 * for translation and rotation of the album art loaded.
 * */
void SquaresWidget::paintGL(){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

    /* m_translate is a float used to keep track of where the coverflow should
     * be translated to. m_translateBuffer is a float used to translate the entire
     * coverflow. m_translateBuffer is incremented or decremented by a relatively
     * small number towards m_translate. This gradual change in m_translateBuffer
     * creates the animation seen in coverflow.
     */
	if(m_translateBuffer != m_translate){
		if(m_translate - m_translateBuffer < -0.001){
			m_translateBuffer -= 0.05;
            m_movingRight = true;
		}
        else m_movingRight = false;
		if(m_translate - m_translateBuffer > 0.001){
			m_translateBuffer += 0.05;
            m_movingLeft = true;
		}
        else m_movingLeft = false;
	}
	glTranslatef(m_translateBuffer,0,0);

    /* If there are no albums loaded into the coverflow, then the OpenGL texture
     * from the loaded default image is displayed on the coverflow. Else, the
     * the OpenGL textures converted from the images of the album art will be
     * rendered and displayed on the coverflow.
     */
    if(m_numAlbums == 0){
        glTranslatef(0,0,m_albumDepth/2);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, m_defTexID);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glBegin(GL_QUADS);
            glTexCoord2f(1, 0);	glVertex3f(1,-1,0);
            glTexCoord2f(0, 0);	glVertex3f(-1,-1,0);
            glTexCoord2f(0, 1);	glVertex3f(-1,1,0);
            glTexCoord2f(1, 1);	glVertex3f(1,1,0);
        glEnd();
        glDisable(GL_TEXTURE_2D);
	}
    else{
        /* m_centerRegion is a float that represents where the center of the
         * coverflow currently is (essentially represents the x-coordinate of the
         * center album art). m_mainAlbum is an integer that represents the index
         * in m_glTexID of the center album art (if there is a center album art).
         * These values also depend on m_numAlbums because a translation is done
         * on the coverflow so that the default album art when music is initially
         * loaded is the album art in the middle.
         */
        if(m_numAlbums%2 == 0){
            m_centerRegion = m_numAlbums*m_shift/2-m_translateBuffer;
            m_mainAlbum = (int)-(m_translateBuffer-0.01)/m_shift+m_numAlbums/2;
        }
        else{
            m_centerRegion = (m_numAlbums-1)*m_shift/2-m_translateBuffer;
            m_mainAlbum = (int)-(m_translateBuffer-0.01)/m_shift+(m_numAlbums-1)/2;
        }
        glTranslatef(-m_shift,0,0);
        if(m_numAlbums%2 == 0) glTranslatef(-m_shift*m_numAlbums/2,0,0);
        else glTranslatef(-m_shift*(m_numAlbums-1)/2,0,0);

        /* This for loop handles the rendering of the album art that will be
         * displayed on the coverflow.
         */
        for(int i = 0; i < m_numAlbums; i++){

            /* A translation is done every iteration in the for loop in order to
             * translate the album art correctly so they do not overlap in the
             * coverflow. The if statement checks whether or not the texture in
             * m_glTexID[i] is at a location relative to the center (main) album
             * that needs to be rendered/displayed on the coverflow.
             */
            glTranslatef(m_shift,0,0);
            if(m_numAlbums > 7 && (i < m_mainAlbum - (m_albumsShown)/2 ||
                i > m_mainAlbum + (m_albumsShown)/2)) continue;

            /* glEnable(GL_TEXTURE_2D) enables the coverflow to use 2d textures.
             * glPushMatrix is used to ensure that the translations and rotations
             * performed on a single album art does not affect the other album art
             * in the coverflow.
             */
            glEnable(GL_TEXTURE_2D);
			glPushMatrix();
			glColor3f(0.8,0.8,0.7);

            /* i*m_shift represents where the album art represented by m_glTexID[i]
             * is relative to m_centerRegion. If the location of the album art is
             * not approximately equal to m_centerRegion, then how much this album
             * art must be rotated and translated will be determined in this if
             * statement. Else, a fixed translation will just be performed on this
             * center album art.
             */
            if(i*m_shift >= m_centerRegion + 0.01
                    || i*m_shift <= m_centerRegion - 0.01){

                /* regionMoved is a float that represents how far things have been
                 * shifted. regionMoved is based on the difference between m_translate
                 * and m_translateBuffer and this difference is relative to m_shift.
                 * It is subtracted by a casted int also in order to ensure that this
                 * number does not become greater than 1 (which is necessary for this
                 * implementation. temp_rotation is 90*regionMoved in order to
                 * properly represent a common angle that will be used in rotating
                 * album art.
                 */
                float regionMoved = ((m_translate-m_translateBuffer)/m_shift-
                                     (int)((m_translate-m_translateBuffer)/m_shift));
                float temp_rotation = 90*regionMoved;

                // If the coverflow is currently moving left, then this if statement
                //is executed
                if(m_movingLeft){

                    /* This if statement checks to see if the position of the album art
                     * being created in this iteration of the for loop is in the region
                     * to the right of the center album that would need to be rotated
                     * as the coverflow is moving. If it is in the region to the right,
                     * then the album art must be translated in the positive
                     * z-direction by m_albumDepth, a constant that can be set to
                     * indicate how far in terms of depth it will be translated, times
                     * regionMoved. It is also rotated based on regionMoved around the
                     * y-axis.
                    */
                    if(i*m_shift >= m_centerRegion + 0.01 && i*m_shift <=
                            m_centerRegion + (m_shift-0.039)){
                        glTranslatef(0,0,m_albumDepth*regionMoved);
                        glRotatef(temp_rotation + 90,0,1,0); //+90
					}

                    /* If, however, the album art is within the region to the left
                     * of the center album art, then this album art must be rotated
                     * and translated based on regionMoved and temp_rotation, similarly
                     * to the previous case.
                     */
                    else if(i*m_shift >= m_centerRegion - (m_shift-0.039) && i*m_shift
                            <= m_centerRegion - 0.01){
                        glTranslatef(0,0,m_albumDepth-m_albumDepth*regionMoved);
						glRotatef(temp_rotation,0,1,0);
					}

                    /* Else, if this album art is outside of the region that needs
                     * to be rotated in an angle based on temp_rotation, the it will
                     * just be rotated 90 degrees around the y-axis.
                     */
					else glRotatef(90,0,1,0);
				}

                /* If the coverflow is currently moving to the right, then, similarly
                 * to the case where coverflow is moving to the left, the album art
                 * is rotated or translated some amount based on regionMoved and
                 * temp_rotation (unless it needs to just be rotated 90 degrees around
                 * the y-axis).
                 */
                else if(m_movingRight){
                    if(i*m_shift >= m_centerRegion + 0.01 && i*m_shift <=
                            m_centerRegion + (m_shift-0.039)){
                        glTranslatef(0,0,m_albumDepth-m_albumDepth*(-1)*regionMoved);
                        glRotatef(temp_rotation + 180,0,1,0);
					}
                    else if(i*m_shift >= m_centerRegion - (m_shift-0.039) && i*m_shift
                            <= m_centerRegion - 0.01){
                        glTranslatef(0,0,m_albumDepth*(-1)*regionMoved);
						glRotatef(temp_rotation + 90,0,1,0);
					}
					else glRotatef(90,0,1,0);
				}
                /* If the coverflow is not moving, then the album art to the left and
                 * right of the center album art needs to be rotated 90 degrees around
                 * the y-axis.
                 */
				else glRotatef(90,0,1,0);
			}

            /* This else statement handles the case in which the album art being
             * rendered is the center album art. Signal s_albumSelected is emitted
             * with the name of the album if this center album was double-clicked on.
             * This passes the name of the album to a function in MainWindow that
             * handles filtering the table in Qtunes. Signal s_currentAlbum is
             * emitted with the name of the album to MainWindow also with the name of
             * the album. This is used to display the name of the album art on the
             * coverflow. Also, a translation is done in the positive z-direction in
             * order to make the center album art essentially rotate outwards (a
             * positive z-direction corresponds to the direction coming out of the
             * screen).
             */
            else{
                if(m_doubleClicked){
                    emit s_albumSelected(m_albumList->at(i));
                    m_doubleClicked = false;
                }
                emit s_currentAlbum(m_albumList->at(i));
                glTranslatef(0,0,m_albumDepth);
            }

            /* This if-else statement actually draws the textures onto the coverflow.
             * This if-else statement is necessary in order to flip the album art
             * if the album art is to the right of the center album art.
             */
            glBindTexture(GL_TEXTURE_2D,m_glTexID->at(i));
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            if(i*m_shift <= m_centerRegion + 0.01){
					glBegin(GL_QUADS);
						glTexCoord2f(1, 0);	glVertex3f(1,-1,0);
						glTexCoord2f(0, 0);	glVertex3f(-1,-1,0);
						glTexCoord2f(0, 1);	glVertex3f(-1,1,0);
						glTexCoord2f(1, 1);	glVertex3f(1,1,0);
					glEnd();
			}
			else{
					glBegin(GL_QUADS);
						glTexCoord2f(1, 0);	glVertex3f(-1,-1,0);
						glTexCoord2f(0, 0);	glVertex3f(1,-1,0);
						glTexCoord2f(0, 1);	glVertex3f(1,1,0);
						glTexCoord2f(1, 1);	glVertex3f(-1,1,0);
					glEnd();
			}
			glDisable(GL_TEXTURE_2D);

            /* This block of code is used to draw a line that will outline each
             * album art drawn in the coverflow with a gold-like color.
             */
			glColor3f(1, (float)215/255, 0);
			glBegin(GL_LINE_LOOP);
				glVertex3f(-1, -1, 0);
				glVertex3f(-1, 1, 0);
				glVertex3f(1, 1, 0);
				glVertex3f(1, -1, 0);
			glEnd();
            glPopMatrix();
        }
	}
    /* glFlush() is needed to essentially draw what was just written above.
     */
    glFlush();
}
