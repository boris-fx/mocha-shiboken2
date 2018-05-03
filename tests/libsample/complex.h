/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the test suite of Qt for Python.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef COMPLEX_H
#define COMPLEX_H

#include "libsamplemacros.h"

class LIBSAMPLE_API Complex
{
public:
    Complex(double real = 0.0, double imag = 0.0);
    ~Complex() {}

    inline double real() const { return m_real; }
    inline void setReal(double real) { m_real = real; }
    inline double imag() const { return m_imag; }
    inline void setImaginary(double imag) { m_imag = imag; }

    Complex operator+(Complex& other);

    void show();

private:
    double m_real;
    double m_imag;
};

#endif // COMPLEX_H

