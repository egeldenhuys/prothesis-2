#include "external_panel.hpp"

#include <memory>
#include <string>

#define CHOICE_BOX_KEYS_COUNT 3

ExternalPanel::ExternalPanel(wxWindow *parent,
                             wxWindowID id,
                             std::string panel_name,
                             std::string panel_title,
                             const wxPoint &pos,
                             const wxSize &size,
                             int64_t style)
  : DataPanel(parent, id, panel_name, panel_title, pos, size, style) {
  sizer_keys_ = new wxBoxSizer(wxVERTICAL);

  DoLayout();
}

ExternalPanel::~ExternalPanel() {
  // void
}


bool ExternalPanel::SetGuiState(std::shared_ptr<cpptoml::table> state) {
  auto panel_table = state->get_table(this->GetPanelName());

  // Get root table from state
  if (panel_table) {
    // Get life keys table
    auto life_keys_table = panel_table->get_table("life_keys");

    if (life_keys_table) {
      // Get type table array
      auto life_key_types = life_keys_table->get_table_array("type");

      if (life_key_types) {
        // Iterate over the types and collect the keys
        wxArrayString keys;

        for (const auto& life_key_type : *life_key_types) {
            auto key = life_key_type->get_as<std::string>("key");

            if (key) {
              // Add the key to the key vector
              keys.Add(wxT(*key));
            } else {
              wxLogDebug(_("'key' was not found for ") +
                         _("table external.life_keys.type ") +
                         _("for panel ") +
                         _(this->GetPanelName()));
            }  // key
        }  // key iteration loop

        // Create choice boxes
        for (size_t i = 0; i < CHOICE_BOX_KEYS_COUNT; i++) {
          choice_boxes_keys_[i] = new wxChoice(this, wxID_ANY,
                                               wxDefaultPosition,
                                               wxSize(-1, -1),
                                               keys);
          sizer_keys_->Add(choice_boxes_keys_[i], 0, 0, 0);
          sizer_keys_->AddSpacer(5);
        }
      } else {
        wxLogDebug(_("Table array external.life_keys.type was not found") +
                   _("for panel ") +
                   _(this->GetPanelName()));
      }  // life_key_types
    } else {
      wxLogDebug(_("Table external.life_keys was not found for panel ") +
                 _(this->GetPanelName()));
    }  // life_keys_table

  } else {
    wxLogDebug(_("No GUI table exists for panel ") + _(this->GetPanelName()));
    return false;
  }  // panel_table

  return true;
}

std::shared_ptr<cpptoml::table> ExternalPanel::GetUserState() {
  auto panel_table = cpptoml::make_table();
  auto life_keys_table = cpptoml::make_table();
  auto life_keys_table_array = cpptoml::make_table_array();

  size_t keys_found = 0;

  // life keys
  for (size_t i = 0; i < CHOICE_BOX_KEYS_COUNT; i++) {
    if (choice_boxes_keys_[i]->GetSelection() != wxNOT_FOUND) {
      auto life_key_type = cpptoml::make_table();
      const int selection = choice_boxes_keys_[i]->GetSelection();
      const std::string key = choice_boxes_keys_[i]->GetString(selection)
        .ToStdString();
      life_key_type->insert("key", key);
      life_keys_table_array->push_back(life_key_type);
      keys_found++;
    }
  }

  if (keys_found > 0) {
    life_keys_table->insert("type", life_keys_table_array);
    panel_table->insert("life_keys", life_keys_table);
  }

  return panel_table;
}

bool ExternalPanel::SetUserState(std::shared_ptr<cpptoml::table> state) {
  auto panel_table = state->get_table(this->GetPanelName());

  // Get root table from state
  if (panel_table) {
    // Get life keys table
    auto life_keys_table = panel_table->get_table("life_keys");

    if (life_keys_table) {
      // Get type table array
      auto life_key_types = life_keys_table->get_table_array("type");

      if (life_key_types) {
        // Iterate over the types and collect the keys
        wxArrayString keys;

        for (const auto& life_key_type : *life_key_types) {
            auto key = life_key_type->get_as<std::string>("key");

            if (key) {
              // Add the key to the key vector
              keys.Add(wxT(*key));
            } else {
              wxLogDebug(_("'key' was not found for ") +
                         _("table external.life_keys.type ") +
                         _("for panel ") +
                         _(this->GetPanelName()));
            }  // key
        }  // key iteration loop

        if (keys.Count() > CHOICE_BOX_KEYS_COUNT) {
          wxLogWarning(_(std::to_string(keys.Count())) +
                         _(" life keys found in user config,") +
                         _(" but only") +
                         _(std::to_string(CHOICE_BOX_KEYS_COUNT)) +
                         _(" are allowed to be selected"));
        }

        // Set choice box selections
        for (size_t i = 0; i < CHOICE_BOX_KEYS_COUNT && i < keys.Count(); i++) {
          const int selection = choice_boxes_keys_[i]->FindString(keys[i]);

          if (selection != wxNOT_FOUND) {
            choice_boxes_keys_[i]->SetSelection(selection);
          } else {
            wxLogWarning(_("The life key ") +
                         _(keys[i]) +
                         _(" was not found as an option in the GUI config"));
          }
        }
      } else {
        wxLogDebug(_("Table array external.life_keys.type was not found") +
                   _("for panel ") +
                   _(this->GetPanelName()));
      }  // life_key_types
    } else {
      wxLogDebug(_("Table external.life_keys was not found for panel ") +
                 _(this->GetPanelName()));
    }  // life_keys_table

  } else {
    wxLogDebug(_("No GUI table exists for panel ") + _(this->GetPanelName()));
    return false;
  }  // panel_table

  return true;
}

void ExternalPanel::DoLayout() {
  wxGridSizer *sizer = new wxGridSizer(1, 2, 0, 0);
  wxBoxSizer *sizer_mbti_keys = new wxBoxSizer(wxVERTICAL);
  wxBoxSizer *sizer_mbti = new wxBoxSizer(wxVERTICAL);
  wxBoxSizer *sizer_mbti_combo_boxes = new wxBoxSizer(wxHORIZONTAL);
  wxBoxSizer *sizer_career = new wxBoxSizer(wxVERTICAL);
  wxPanel *panel_career = new wxPanel(this, wxID_ANY);
  panel_career->SetBackgroundColour(wxColour(255, 0, 0));
  panel_career->SetMinSize(wxSize(200, 300));
  sizer_career->Add(panel_career, 0, wxEXPAND, 0);

  wxStaticText *label_mbti = new wxStaticText(this, wxID_ANY, _("MBTI"));
  sizer_mbti->Add(label_mbti, 0, wxALIGN_BOTTOM, 0);

  wxChoice *choice_boxes_mbti[4];
  for (size_t i = 0; i < 4; i++) {
    choice_boxes_mbti[i] = new wxChoice(this, wxID_ANY, wxDefaultPosition,
                                  wxSize(60, -1));
    sizer_mbti_combo_boxes->Add(choice_boxes_mbti[i], 0, 0, 0);
    sizer_mbti_combo_boxes->AddSpacer(5);
  }
  sizer_mbti->Add(sizer_mbti_combo_boxes, 0, 0, 0);

  wxStaticText *label_keys = new wxStaticText(this, wxID_ANY, _("Life Keys"));
  sizer_keys_->Add(label_keys, 0, wxALIGN_BOTTOM, 0);

  sizer_mbti_keys->Add(sizer_mbti, 0, wxBOTTOM, 20);
  sizer_mbti_keys->Add(sizer_keys_, 0, 0, 0);

  sizer->Add(sizer_mbti_keys, 0, 0, 0);
  sizer->Add(sizer_career, 0, 0, 0);

  Layout();
  this->SetSizer(sizer);
}
