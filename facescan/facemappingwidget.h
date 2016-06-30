#ifndef FACEMAPPINGWIDGET_H
#define FACEMAPPINGWIDGET_H

#include "FaceMappingEngine.h"

class FaceMappingWidget: public QWidget
{
    Q_OBJECT

private:
    FaceMappingEngine* mFMEngine;

public:
    FaceMappingWidget();
    ~FaceMappingWidget();
    QSize sizeHint() const;

protected:


public slots:
    void setHairIndex(int idx);
    void loadFace(QString path);

};

#endif // FACEMAPPINGWIDGET_H
