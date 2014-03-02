#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
	//set default path
	path="TrackedData";
	//TODO: default look so that user does not press any other button
	ui->setupUi(this);
    list=NULL;
	//make/check the required directories
	if(!QDir(QString(path.c_str())).exists())
		QDir().mkdir(QString(path.c_str()));
	//timer
	timer = new QTimer(this);
	timer_interval= DEFAULT_TIMER_INTERVAL;
	current_frame=0;
	sceneset=false;
	modelloaded=false;
	//hide the groupboxes
	ui->DataSetGroup->setVisible(false);
	ui->ModelSetGroup->setVisible(false);
	ui->loadnewbutton->setVisible(false);
	ui->loadnewmodelbutton->setVisible(false);

	//default values
	datatranslation=cv::Point3f(0,0,0);
	modeltranslation=cv::Point3f(3950,-280,0);
	modelrotation=cv::Point3f(0,-90,0);
	tempmodelmovement=cv::Point3f(0,0,0);
	slideron=false;
	
	//connect the slots
	//buttons
	connect(ui->LoadButton,SIGNAL(clicked()),this,SLOT(load_button_click()));
	connect(ui->LoadModelButton,SIGNAL(clicked()),this,SLOT(load_model_click()));
	//data xyz buttons
	connect(ui->xdataslider,SIGNAL(sliderMoved(int)),this,SLOT(xslidervalue(int)));
	connect(ui->xdataslider,SIGNAL(sliderReleased()),this,SLOT(snapback()));
	connect(ui->ydataslider,SIGNAL(sliderMoved(int)),this,SLOT(yslidervalue(int)));
	connect(ui->ydataslider,SIGNAL(sliderReleased()),this,SLOT(snapback()));
	connect(ui->zdataslider,SIGNAL(sliderMoved(int)),this,SLOT(zslidervalue(int)));
	connect(ui->zdataslider,SIGNAL(sliderReleased()),this,SLOT(snapback()));
	//data checkboxes
	connect(ui->showspherecbox,SIGNAL(stateChanged(int)),this,SLOT(toggleshowspheres(int)));
	connect(ui->bgcheckbox,SIGNAL(stateChanged(int)),this,SLOT(togglebackground(int)));
	//model xyz buttons
	connect(ui->xmodelslider,SIGNAL(sliderMoved(int)),this,SLOT(modelxslidervalue(int)));
	connect(ui->xmodelslider,SIGNAL(sliderReleased()),this,SLOT(snapback()));
	connect(ui->ymodelslider,SIGNAL(sliderMoved(int)),this,SLOT(modelyslidervalue(int)));
	connect(ui->ymodelslider,SIGNAL(sliderReleased()),this,SLOT(snapback()));
	connect(ui->zmodelslider,SIGNAL(sliderMoved(int)),this,SLOT(modelzslidervalue(int)));
	connect(ui->zmodelslider,SIGNAL(sliderReleased()),this,SLOT(snapback()));
	
	connect(ui->rotxmodelslider,SIGNAL(sliderMoved(int)),this,SLOT(modelrotxslidervalue(int)));
	connect(ui->rotxmodelslider,SIGNAL(sliderReleased()),this,SLOT(snapback()));
	connect(ui->rotymodelslider,SIGNAL(sliderMoved(int)),this,SLOT(modelrotyslidervalue(int)));
	connect(ui->rotymodelslider,SIGNAL(sliderReleased()),this,SLOT(snapback()));
	connect(ui->rotzmodelslider,SIGNAL(sliderMoved(int)),this,SLOT(modelrotzslidervalue(int)));
	connect(ui->rotzmodelslider,SIGNAL(sliderReleased()),this,SLOT(snapback()));

	//model checkbox
	connect(ui->showmodelcheckbox,SIGNAL(stateChanged(int)),this,SLOT(togglemodel(int)));
	connect(ui->loadnewbutton,SIGNAL(clicked()),this,SLOT(delete_button_click()));
	connect(ui->loadnewmodelbutton,SIGNAL(clicked()),this,SLOT(delete_model_click()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void MainWindow::load_button_click()
{
	//if(sceneset){
	//	ui->DisplayWidget3D->delete_all();
	//	cv::waitKey(1000);
	//	delete process;
	//}
	//Open the dialog box
	QString fileName = QFileDialog::getOpenFileName(this);
	//Filter all raw files
	QDir raw_folder=QFileInfo(fileName).absoluteDir();
	raw_folder.setNameFilters(QStringList()<<"*.raw");
	raw_folder.setSorting(QDir::Name);
	QStringList fileList = raw_folder.entryList();
	//int num_files= fileList.length();
	//Find first and last numbers (Assume sequential capture)
	QStringList templist=fileList[0].split("/");
	QString fname=templist[templist.size()-1];
	fname.chop(4);
	int first_num=fname.toInt();
	templist=fileList[fileList.size()-1].split("/");
	fname=templist[templist.size()-1];
	fname.chop(4);
	int last_num=fname.toInt();
	string abspath=std::string(raw_folder.absolutePath().toLocal8Bit().constData());
	//pass to raw reader
	rawReader reader;
	reader.readfile(abspath,first_num,last_num,RAWWIDTH,RAWHEIGHT);
	int no_of_images=last_num-first_num+1;
	process=new Process(reader.data,no_of_images,RAWWIDTH,RAWHEIGHT);
	process->make_contours();
	//process.view();
	ui->DisplayWidget3D->draw_data(process->return_root());
	sceneset=true;
	ui->LoadButton->setVisible(false);
	ui->loadnewbutton->setVisible(true);
	ui->DataSetGroup->setVisible(true);

	//if (!fileName.isEmpty())
	//{
	//	//try
	//	{
	//		
	//		//track= new Tracker(std::string(fileName.toLocal8Bit().constData()),true);
	//		QStringList filenameList=fileName.split("/");
	//		fileName=filenameList[filenameList.size()-2];
	//		fileName.chop(4);
	//		path+="/"+std::string(fileName.toLocal8Bit().constData());
	//		origpath=path+"/"+"Original";
	//		cleanedpath=path+"/"+"Cleaned";
	//		contpath=path+"/"+"Contours";
	//		immpath=path+"/"+"Immersive";
	//		
	//		//if(!QDir(QString(path.c_str())).exists())
	//		//{
	//		//	QDir().mkdir(QString(path.c_str()));
	//		//	if(!QDir(QString(origpath.c_str())).exists())
	//		//	{
	//		//		QDir().mkdir(QString(origpath.c_str()));
	//		//		for(int i=0;i<imgs.size();i++)
	//		//		{
	//		//			char num[5];
	//		//			sprintf(num,"%03d",i);
	//		//			string fpath=origpath+"/"+string(num)+PICFORMAT;
	//		//			cv::imwrite(fpath,imgs[i]);
	//		//		}
	//		//	}
	//		//}


	//	}
	//	/*catch (MyException& e)
	//	{
	//		QMessageBox msgBox;
	//		msgBox.setText(e.what());
	//		msgBox.exec();
	//	}*/
	//}
	//else
	//{
	//	QMessageBox msgBox;
	//	msgBox.setText("Please choose a movie file of the required data");
	//	msgBox.exec();
	//}

}

void MainWindow::load_model_click()
{
	modelfileName = QFileDialog::getOpenFileName(this);
	ui->DisplayWidget3D->draw_model(string(modelfileName.toLocal8Bit().constData()));
	ui->LoadModelButton->setVisible(false);
	ui->loadnewmodelbutton->setVisible(true);
	ui->ModelSetGroup->setVisible(true);
	modelloaded=true;
}

void MainWindow::delete_button_click()
{
	ui->DisplayWidget3D->delete_data();
	ui->LoadButton->setVisible(true);
	ui->loadnewbutton->setVisible(false);
	ui->DataSetGroup->setVisible(false);
	datatranslation=cv::Point3f(0,0,0);
	delete process;
}

void MainWindow::delete_model_click()
{
	ui->DisplayWidget3D->delete_model();
	ui->LoadModelButton->setVisible(true);
	ui->ModelSetGroup->setVisible(false);
	ui->loadnewmodelbutton->setVisible(false);
	modeltranslation=cv::Point3f(0,0,0);
}
QImage Mat2QImage(const cv::Mat3b &src) {
        QImage dest(src.cols, src.rows, QImage::Format_ARGB32);
        for (int y = 0; y < src.rows; ++y) {
                const cv::Vec3b *srcrow = src[y];
                QRgb *destrow = (QRgb*)dest.scanLine(y);
                for (int x = 0; x < src.cols; ++x) {
                        destrow[x] = qRgba(srcrow[x][2], srcrow[x][1], srcrow[x][0], 255);
                }
        }
        return dest;
}
void MyGLDrawer::drawImage(QImage& image)
{
	/*glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,num);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D (GL_TEXTURE_2D, 0, 3, image.width(), image.height(), 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE,image.bits());*/
	QImage imagerotated=image.copy();
	imagerotated.scaled(QSize(this->width(),this->height()));
	buffer=imagerotated.mirrored();
	updateGL();
}
void MyGLDrawer::paintGL()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	/*glTexCoord2f(0.0,0.0); glVertex2f(-1,-1);
	glTexCoord2f(0.0,1.0); glVertex2f(1,-1);
	glTexCoord2f(1.0,1.0); glVertex2f(1,1);
	glTexCoord2f(1.0,0.0); glVertex2f(-1,1);
	glDisable(GL_TEXTURE_2D);*/
	glDrawPixels(buffer.width(), buffer.height(), GL_RGBA, GL_UNSIGNED_BYTE, buffer.bits());
}


void MainWindow::xslidervalue(int value){
	datatranslation.x-=tempmodelmovement.x;
	tempmodelmovement.x=float(value-50)*SLIDERSTEP*2;
	datatranslation.x+=tempmodelmovement.x;
	ui->DisplayWidget3D->translate_data_x(datatranslation.x);
	ui->dataxposlabel->setText("X: "+QString::number(datatranslation.x));
}
void MainWindow::yslidervalue(int value){
	datatranslation.y-=tempmodelmovement.y;
	tempmodelmovement.y=float(value-50)*SLIDERSTEP*2;
	datatranslation.y+=tempmodelmovement.y;
	ui->DisplayWidget3D->translate_model_y(datatranslation.y);
	ui->datayposlabel->setText("Y: "+QString::number(datatranslation.y));
}
void MainWindow::zslidervalue(int value){
	datatranslation.z-=tempmodelmovement.z;
	tempmodelmovement.z=float(value-50)*SLIDERSTEP*2;
	datatranslation.z+=tempmodelmovement.z;
	ui->DisplayWidget3D->translate_data_z(datatranslation.z);
	ui->datazposlabel->setText("Z: "+QString::number(datatranslation.z));
}
void MainWindow::snapback(){

	ui->xdataslider->setSliderPosition(50);
	ui->ydataslider->setSliderPosition(50);
	ui->zdataslider->setSliderPosition(50);
	ui->xmodelslider->setSliderPosition(50);
	ui->ymodelslider->setSliderPosition(50);
	ui->zmodelslider->setSliderPosition(50);
	ui->rotxmodelslider->setSliderPosition(50);
	ui->rotymodelslider->setSliderPosition(50);
	ui->rotzmodelslider->setSliderPosition(50);

	tempmodelmovement=cv::Point3f(0,0,0);
	slideron=false;
}

void MainWindow::modelxslidervalue(int value){
	modeltranslation.x-=tempmodelmovement.x;
	tempmodelmovement.x=float(value-50)*SLIDERSTEP*2;
	modeltranslation.x+=tempmodelmovement.x;
	ui->DisplayWidget3D->translate_model_x(modeltranslation.x);
	ui->modelxposlabel->setText("X: "+QString::number(modeltranslation.x));
}
void MainWindow::modelyslidervalue(int value){
	modeltranslation.y-=tempmodelmovement.y;
	tempmodelmovement.y=float(value-50)*SLIDERSTEP*2;
	modeltranslation.y+=tempmodelmovement.y;
	ui->DisplayWidget3D->translate_model_y(modeltranslation.y);
	ui->modelyposlabel->setText("Y: "+QString::number(modeltranslation.y));
}
void MainWindow::modelzslidervalue(int value){
	modeltranslation.z-=tempmodelmovement.z;
	tempmodelmovement.z=float(value-50)*SLIDERSTEP*2;
	modeltranslation.z+=tempmodelmovement.z;
	ui->DisplayWidget3D->translate_model_z(modeltranslation.z);
	ui->modelzposlabel->setText("Z: "+QString::number(modeltranslation.z));
}

void MainWindow::togglemodel(int value){
	ui->DisplayWidget3D->switch_model(bool(value));
}

void MainWindow::modelrotxslidervalue(int value){
	if(slideron)
		ui->DisplayWidget3D->rotate_model_x(-tempmodelmovement.x);
	modelrotation.x-=tempmodelmovement.x;
	tempmodelmovement.x=float(value-50)*3.6;
	modelrotation.x+=tempmodelmovement.x;
	ui->DisplayWidget3D->rotate_model_x(tempmodelmovement.x);
	ui->modelxrotlabel->setText("X: "+QString::number(modelrotation.x));
	slideron=true;
}

void MainWindow::modelrotyslidervalue(int value){
	if(slideron)
		ui->DisplayWidget3D->rotate_model_y(-tempmodelmovement.y);
	modelrotation.y-=tempmodelmovement.y;
	tempmodelmovement.y=float(value-50)*3.6;
	modelrotation.y+=tempmodelmovement.y;
	ui->DisplayWidget3D->rotate_model_y(tempmodelmovement.y);
	ui->modelyrotlabel->setText("Y: "+QString::number(modelrotation.y));
	slideron=true;
}

void MainWindow::modelrotzslidervalue(int value){
	if(slideron)
		ui->DisplayWidget3D->rotate_model_z(-tempmodelmovement.z);
	modelrotation.z-=tempmodelmovement.z;
	tempmodelmovement.z=float(value-50)*3.6;
	modelrotation.z+=tempmodelmovement.z;
	ui->DisplayWidget3D->rotate_model_z(tempmodelmovement.z);
	ui->modelzrotlabel->setText("Z: "+QString::number(modelrotation.z));
	slideron=true;
}