//--------------------------------------------------------------------------------------
// Copyright 2011 Intel Corporation
// All Rights Reserved
//
// Permission is granted to use, copy, distribute and prepare derivative works of this
// software for any purpose and without fee, provided, that the above copyright notice
// and this statement appear in all copies.  Intel makes no representations about the
// suitability of this software for any purpose.  THIS SOFTWARE IS PROVIDED "AS IS."
// INTEL SPECIFICALLY DISCLAIMS ALL WARRANTIES, EXPRESS OR IMPLIED, AND ALL LIABILITY,
// INCLUDING CONSEQUENTIAL AND OTHER INDIRECT DAMAGES, FOR THE USE OF THIS SOFTWARE,
// INCLUDING LIABILITY FOR INFRINGEMENT OF ANY PROPRIETARY RIGHTS, AND INCLUDING THE
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  Intel does not
// assume any responsibility for any errors which may appear in this software nor any
// responsibility to update it.
//--------------------------------------------------------------------------------------

#include <QApplication>
#include "facemappingwidget.h"

#include <QPropertyAnimation>

#include <QTimer>

int main(int argc, char **argv)
{
    QApplication a(argc,argv);

    FaceMappingWidget fmw;
    fmw.show();

    //because we can!
    QPropertyAnimation animation(&fmw, "postBMI");
    animation.setDuration(2000);
    animation.setStartValue(0.0);
    animation.setEndValue(1.0);
    QObject::connect(&animation, &QPropertyAnimation::finished, [&]{
        if (animation.direction() == QPropertyAnimation::Backward) {
            animation.setDirection(QPropertyAnimation::Forward);
        } else {
            animation.setDirection(QPropertyAnimation::Backward);
        }
        animation.start();
    });

    QTimer::singleShot(0,[&] {
        fmw.loadFace(a.applicationDirPath() + "/userdata/joe_sr300_1.obj");

        /*  0 ("Front Profile", "Head Width", 0.5f
         *  1 ("Front Profile", "Eye Area Width", 0.5f
         *  2 ("Front Profile", "Cheekbone Width", 0.5f
         *  3 ("Front Profile", "OCC Width", 0.5f
         *  4 ("Front Profile", "Jaw Width", 0.5f
         *  5 ("Front Profile", "Jaw Level", 0.5f
         *  6 ("Front Profile", "Chin Width", 0.5f
         *  7 ("Front Profile", "Neck Width", 0.5f
         *  8 ("Base Shape", "Shape 1", 0.0f
         *  9 ("Base Shape", "Shape 2", 0.0f
         *  10 ("Base Shape", "Shape 3", 0.0f
         *  11 ("Base Shape", "Width", 0.0f
         *  12 ("Base Shape", "Roundness", 0.5f
         *  13 ("Base Shape", "BMI", 0.5f
         *  14 ("Jaw", "Cheekbone", 0.0f, "shape_Cheekbone_Size", 0.0f, 1.0f, 0.0f, 1.0f);
         *  15 ("Jaw", "Chin Protrude", 0.5f
         *  16 ("Jaw", "Chin Level", 0.0f
         *  17 ("Jaw", "Chin Width", 0.5f
         *  18 ("Other", "Neck Slope", 0.0f
        */
        fmw.setMorphParamWeight(5, 0.8f);

        // fmw.setBlendColor1(QColor(10,10,10));
        //    fmw.setPostBMI(0.8);
        //    fmw.setPostOgre(0.8);
        //    fmw.setFaceOrientation(0.f,0.f,0.f);
        //    fmw.setFaceZOffset(-3.5f);
        fmw.setHairIndex(4);

        animation.start();
    });

    return a.exec();
}
