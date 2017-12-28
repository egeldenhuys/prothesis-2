#include "data_manager.hpp"
#include <string>
#include <memory>
#include <sstream>
#include <fstream>
#include <vector>
#include "panels/details_panel.hpp"
#include "questions_panel.hpp"

DataManager::DataManager(wxFrame *main_frame) {
  main_frame_ = main_frame;

  std::string base_path = GetBasePath();
  wxLogDebug(_(std::string("base_path = ") + base_path));

  user_config_path_ = base_path + user_config_path_;
  wxLogDebug(_(std::string("user_config_path_ = ") + user_config_path_));

  gui_config_path_ = base_path + gui_config_path_;
  wxLogDebug(_(std::string("gui_config_path_ = ") + gui_config_path_));

  for (size_t i = 0; i < PanelId::kPanelCount; i++) {
    panels_[i] = NULL;
  }

  DeclarePanels();
  Load();
}

void DataManager::AddPanel(DataPanel* panel, PanelId id) {
  wxLogDebug(_("Registering panel: ") + _(panel->GetPanelName()));

  panels_[id] = panel;
  ids_[id] = id;

  panel->Hide();
}

void DataManager::DeclarePanels() {
  // FLAG: new-panel

  // DETAILS
  AddPanel(new DetailsPanel(main_frame_, wxID_ANY,
                            std::string("details"),
                            std::string("Details")),
           PanelId::kDetailsPanel);
  // PASSIONS
  AddPanel(new QuestionsPanel(main_frame_, wxID_ANY,
                              std::string("passion"),
                              std::string("Passion")),
           PanelId::kPassionPanel);
  // PEOPLE ID
  AddPanel(new QuestionsPanel(main_frame_, wxID_ANY,
                              std::string("people_id"),
                              std::string("People ID")),
           PanelId::kPeopleIdPanel);
  // DREAMS
  AddPanel(new QuestionsPanel(main_frame_, wxID_ANY,
                              std::string("dreams"),
                              std::string("Dreams")),
           PanelId::kDreamsPanel);
  // VALUES
  AddPanel(new QuestionsPanel(main_frame_, wxID_ANY,
                              std::string("values"),
                              std::string("Values")),
           PanelId::kValuesPanel);
  // SPOKEN WORDS
  AddPanel(new QuestionsPanel(main_frame_, wxID_ANY,
                              std::string("spoken_words"),
                              std::string("Spoken Words")),
           PanelId::kSpokenWordsPanel);
}

// TODO(egeldenhuys): Handle parse errors
void DataManager::Load() {
  bool gui_config_exists = Utilities::FileExists(gui_config_path_);
  bool user_config_exists = Utilities::FileExists(user_config_path_);

  if (gui_config_exists) {
    std::shared_ptr<cpptoml::table> gui_config =
      cpptoml::parse_file(gui_config_path_);

    for (size_t i = 0; i < PanelId::kPanelCount; i++) {
      panels_[i]->SetGuiState(gui_config);
    }
  } else {
    wxLogDebug(_("The GUI config file does not exist: ") + _(gui_config_path_));
    return;
  }

  if (user_config_exists) {
    std::shared_ptr<cpptoml::table> user_config =
      cpptoml::parse_file(user_config_path_);
    for (size_t i = 0; i < PanelId::kPanelCount; i++) {
      panels_[i]->SetUserState(user_config);
    }
  } else {
    wxLogDebug(_("The User config file does not exist: ") +
               _(user_config_path_));
  }
}

DataManager::PanelId DataManager::GetIdFromIndex(size_t index) {
  return ids_[index];
}

DataPanel* DataManager::GetPanelByIndex(size_t index) {
  return panels_[index];
}

DataPanel* DataManager::GetPanelById(DataManager::PanelId panel_id) {
  return panels_[panel_id];
}

void DataManager::SaveUserConfig() {
  wxLogDebug("Saving user data...");

  std::shared_ptr<cpptoml::table> user_config = cpptoml::make_table();
  try {
    std::shared_ptr<cpptoml::table> user_config =
      cpptoml::make_table();

    for (size_t i = 0; i < PanelId::kPanelCount; i++) {
      wxLogDebug(_("Getting state for panel ") + _(panels_[i]->GetPanelName()));
      std::shared_ptr<cpptoml::table> panel_config =
        panels_[i]->GetUserState();
      if (panel_config) {
        user_config->insert(panels_[i]->GetPanelName(), panel_config);
      }
    }

    std::stringstream output;
    output << *user_config;
    wxLogDebug("Writing User config...");

    std::fstream fs(user_config_path_, std::fstream::out);
    fs << output.str();
    fs.close();
    wxLogDebug("Done.");
  } catch (std::exception &e) {
    wxLogDebug("Failed to save user config");
    wxLogDebug(e.what());
  } catch (...) {
    wxLogDebug("SaveUserConfig() FAILED");
  }
}

// TODO(egeldenhuys): MacOS
std::string DataManager::GetBasePath() {
  // I might be mistaken but seeing as macOs is considered
  // to be a nix* os this should trigger when compiling on
  // macos.
  #ifdef __unix__
       // Find the relative path using argv[0]
       // TODO(egeldenhuys): Absolute path
       std::vector<std::string> tokens =
         Utilities::SplitString(std::string(wxTheApp->argv[0]), '/');
       std::string base_path = "";
       for (size_t i = 0; i < tokens.size() - 1; i++) {
         base_path.append(tokens.at(i));
         base_path.append("/");
       }
       return base_path;
  #elif _WIN32
       // https://stackoverflow.com/questions/2647429/c-windows-
       // path-to-the-folder-where-the-executable-is-located
       HMODULE hModule = GetModuleHandleW(NULL);
       WCHAR path[MAX_PATH];
       GetModuleFileNameW(hModule, path, MAX_PATH);

       // Use GetCurrentDirectory()
       std::wstring ws(path);
       std::string str(ws.begin(), ws.end());
       std::string base_path = "";
       std::vector<std::string> tokens = Utilities::SplitString(str, '\\');
       for (size_t i = 0; i < tokens.size() - 1; i++) {
         base_path.append(tokens.at(i));
         base_path.append("/");
       }
       return base_path;
  #endif

  throw std::logic_error("UNKNOWN OPERATING SYSTEM");
}

DataManager::~DataManager() {
  // wxLogDebug("DataManager::~DataManager()");
}
