/*
 * Copyright (C) 2006-2018 Istituto Italiano di Tecnologia (IIT)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "yscopewindow.h"
#include "ui_yscopewindow.h"

YscopeWindow::YscopeWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::YscopeWindow)
{
    ui->setupUi(this);
    index = 0;
    connect(ui->cancelBtn,SIGNAL(clicked()),this,SLOT(onCancel()));
    connect(ui->inspectBtn,SIGNAL(clicked()),this,SLOT(onInspect()));
    connect(ui->spinBox,SIGNAL(valueChanged(int)),this,SLOT(onIndexChanged(int)));
}

YscopeWindow::~YscopeWindow()
{
    delete ui;
}

void YscopeWindow::onCancel()
{
    reject();
}

void YscopeWindow::onInspect()
{
    accept();
}

void YscopeWindow::onIndexChanged(int val)
{
    index = val;
}

int YscopeWindow::getIndex()
{
    return index;
}
