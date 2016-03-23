// Copyright (C) 2005-2010 Valeriy Argunov (nporep AT mail DOT ru)
/*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "app.h"
#include <iostream>
#include <fstream>

IMPLEMENT_APP(QSPApp)

std::ofstream log_stream;
wxLogStream * logger;
wxLogFormatter * formatter;

bool QSPApp::OnInit()
{
	wxLog::EnableLogging(true);

	log_stream.open("log_qspgui.txt");
	logger = new wxLogStream(&log_stream);
	//logger->SetTimestamp("%Y-%m-%d %H:%M:%S");
	//formatter = logger->SetFormatter(new wxLogFormatter());
	wxHandleFatalExceptions(true);
	wxInitAllImageHandlers();
	QSPInit();
	logger->LogText("qsp initialized");

	try { InitUI(); }
	catch (...)
	{
		logger->LogText("exception during ui initialization.");
	}

	logger->LogText("ui initialized");
	return true;
}

void QSPApp::OnFatalException()
{
	logger->LogText("fatal exception happened.");
	this->OnExit();
};

int QSPApp::OnExit()
{
	logger->LogText("Uninitializing ...");

	QSPDeInit();
	QSPCallBacks::DeInit();
	delete m_transhelper;
	wxTheClipboard->Flush();
	
	logger->Flush();
	delete logger;

	log_stream.close();
	return wxApp::OnExit();
}

void QSPApp::InitUI()
{
	wxString appPathString = QSPTools::GetAppPath();
	m_transhelper = new QSPTranslationHelper(QSP_APPNAME, appPathString + wxT("langs"));
	wxString configPath = appPathString + QSP_CONFIG;
	if (!(wxFileExists(configPath) || wxFileName::IsDirWritable(appPathString)))
		configPath = wxFileName(wxStandardPaths::Get().GetUserConfigDir(), QSP_CONFIG).GetFullPath();
	logger->LogText("files initialized.");
	// ----------------------
	QSPFrame * frame = new QSPFrame(configPath, m_transhelper);
	logger->LogText("frame created.");
	frame->LoadSettings();
	logger->LogText("frame: settings applied.");
	frame->EnableControls(false);
	QSPCallBacks::Init(frame);
	logger->LogText("frame initialized.");
	// ----------------------
	wxInitEvent initEvent;
	if (GetAutoRunEvent(initEvent))
		wxPostEvent(frame, initEvent);
	logger->LogText("events initialized.");
}

bool QSPApp::GetAutoRunEvent(wxInitEvent& initEvent)
{
	wxCmdLineParser cmdParser(argc, argv);
	if (argc > 1)
	{
		cmdParser.AddParam();
		cmdParser.Parse(false);
		wxFileName path(cmdParser.GetParam());
		path.MakeAbsolute();
		initEvent.SetInitString(path.GetFullPath());
		return true;
	}
	else
	{
		wxFileName autoPath(wxT("auto.qsp"));
		autoPath.MakeAbsolute();
		wxString autoPathString(autoPath.GetFullPath());
		if (wxFileExists(autoPathString))
		{
			initEvent.SetInitString(autoPathString);
			return true;
		}
	}
	return false;
}
