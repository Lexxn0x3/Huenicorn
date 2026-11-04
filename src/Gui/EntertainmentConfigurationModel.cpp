// Huenicorn headers (pulling in Crow) must come before Qt headers because of
// the Qt "signals" macro clashing with Crow's signals() method
#include <Huenicorn/HuenicornCore.hpp>

#include <algorithm>

#include <Huenicorn/Gui/EntertainmentConfigurationModel.hpp>


namespace Huenicorn
{
  EntertainmentConfigurationModel::EntertainmentConfigurationModel(HuenicornCore* core, QObject* parent):
  QAbstractListModel(parent),
  m_core(core)
  {}


  int EntertainmentConfigurationModel::rowCount(const QModelIndex& parent) const
  {
    if(parent.isValid()){
      return 0;
    }

    return static_cast<int>(m_entries.size());
  }


  QVariant EntertainmentConfigurationModel::data(const QModelIndex& index, int role) const
  {
    if(!index.isValid() || index.row() < 0 || index.row() >= static_cast<int>(m_entries.size())){
      return {};
    }

    const Entry& entry = m_entries.at(index.row());

    switch(role){
      case IdRole:      return entry.id;
      case NameRole:    return entry.name;
      case CurrentRole: return entry.current;
      default:          return {};
    }
  }


  QHash<int, QByteArray> EntertainmentConfigurationModel::roleNames() const
  {
    return {
      {IdRole,      "id"},
      {NameRole,    "name"},
      {CurrentRole, "current"}
    };
  }


  void EntertainmentConfigurationModel::reload()
  {
    beginResetModel();

    m_entries.clear();

    if(m_core && m_core->running()){
      const EntertainmentConfigurations& configurations = m_core->entertainmentConfigurations();
      std::optional<std::string> currentId = m_core->currentEntertainmentConfigurationId();

      std::vector<const EntertainmentConfigurations::value_type*> sorted;
      sorted.reserve(configurations.size());
      for(const auto& entry : configurations){
        sorted.push_back(&entry);
      }
      std::sort(sorted.begin(), sorted.end(), [](const auto* a, const auto* b){
        return a->second.name < b->second.name;
      });

      for(const auto* entry : sorted){
        Entry modelEntry;
        modelEntry.id = QString::fromStdString(entry->first);
        modelEntry.name = QString::fromStdString(entry->second.name);
        modelEntry.current = currentId.has_value() && currentId.value() == entry->first;
        m_entries.append(modelEntry);
      }
    }

    endResetModel();
  }
}
