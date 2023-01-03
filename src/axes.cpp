#include <FL/Fl.H>
#include <FL/Fl_Gl_Window.H>
#include <FL/gl.h>
#include <FL/glu.h>

class CoordinateAxesWindow : public Fl_Gl_Window
{
public:
    CoordinateAxesWindow(int x, int y, int w, int h, const char *label = nullptr)
        : Fl_Gl_Window(x, y, w, h, label), m_drag_start_x(0), m_drag_start_y(0), m_dragging(false)
    {
    }

protected:
    void draw() override
    {
        if (!valid())
        {
            glLoadIdentity();
            glViewport(0, 0, w(), h());
            gluOrtho2D(-1, 1, -1, 1);
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        gluLookAt(0, 0, 3, 0, 0, 0, 0, 1, 0);

        // Draw coordinate axes
        glBegin(GL_LINES);
        glColor3f(1, 0, 0);
        glVertex3f(0, 0, 0);
        glVertex3f(1, 0, 0);
        glColor3f(0, 1, 0);
        glVertex3f(0, 0, 0);
        glVertex3f(0, 1, 0);
        glColor3f(0, 0, 1);
        glVertex3f(0, 0, 0);
        glVertex3f(0, 0, 1);
        glEnd();
    }

    int handle(int event) override
    {
        switch (event)
        {
        case FL_PUSH:
            m_drag_start_x = Fl::event_x();
            m_drag_start_y = Fl::event_y();
            m_dragging = true;
            return 1;
        case FL_DRAG:
        {
            if (!m_dragging)
            {
                return 0;
            }
            int dx = Fl::event_x() - m_drag_start_x;
            int dy = Fl::event_y() - m_drag_start_y;
            // Update camera pose based on drag delta
            redraw();
            return 1;
        }
        case FL_RELEASE:
            m_dragging = false;
            return 1;
        default:
            return Fl_Gl_Window::handle(event);
        }
    }

private:
    int m_drag_start_x, m_drag_start_y;
    bool m_dragging;
};

int main(int argc, char **argv)
{
    Fl_Window *window = new Fl_Window(640, 480, "Coordinate Axes");
    CoordinateAxesWindow *gl_window = new CoordinateAxesWindow(10, 10, 620, 460);
    window->end();
    window->show(argc, argv);
    return Fl::run();
}