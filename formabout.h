#ifndef FORMABOUT_H
#define FORMABOUT_H

#include <QWidget>

namespace Ui {
class FormAbout;
}

class FormAbout : public QWidget
{
    Q_OBJECT
    
public:
    explicit FormAbout(QWidget *parent = 0);
    ~FormAbout();
    
private:
    Ui::FormAbout *ui;
};

#endif // FORMABOUT_H
