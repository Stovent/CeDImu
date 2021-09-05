#include "SettingsFrame.hpp"
#include "MainFrame.hpp"
#include "../Config.hpp"

SettingsFrame::SettingsFrame(MainFrame* parent) :
    wxFrame(parent, wxID_ANY, "Settings", wxDefaultPosition, wxSize(450, 315)),
    m_mainFrame(parent)
{
    Show();
}

SettingsFrame::~SettingsFrame()
{
    m_mainFrame->m_settingsFrame = nullptr;
}
