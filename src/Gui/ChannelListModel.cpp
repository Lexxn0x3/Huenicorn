// Huenicorn headers (pulling in Crow) must come before Qt headers because of
// the Qt "signals" macro clashing with Crow's signals() method
#include <Huenicorn/Channel.hpp>
#include <Huenicorn/HuenicornCore.hpp>

#include <algorithm>

#include <Huenicorn/Gui/ChannelListModel.hpp>


namespace Huenicorn
{
  ChannelListModel::ChannelListModel(HuenicornCore* core, QObject* parent):
  QAbstractListModel(parent),
  m_core(core)
  {}


  int ChannelListModel::rowCount(const QModelIndex& parent) const
  {
    if(parent.isValid()){
      return 0;
    }

    return static_cast<int>(m_entries.size());
  }


  QVariant ChannelListModel::data(const QModelIndex& index, int role) const
  {
    if(!index.isValid() || index.row() < 0 || index.row() >= static_cast<int>(m_entries.size())){
      return {};
    }

    const Entry& entry = m_entries.at(index.row());

    switch(role){
      case ChannelIdRole:   return entry.channelId;
      case ActiveRole:      return entry.active;
      case GammaFactorRole: return entry.gammaFactor;
      case UvMinXRole:      return entry.uvMinX;
      case UvMinYRole:      return entry.uvMinY;
      case UvMaxXRole:      return entry.uvMaxX;
      case UvMaxYRole:      return entry.uvMaxY;
      case DevicesRole:     return entry.devices;
      case LabelRole:       return entry.label;
      default:              return {};
    }
  }


  QHash<int, QByteArray> ChannelListModel::roleNames() const
  {
    return {
      {ChannelIdRole,   "channelId"},
      {ActiveRole,      "active"},
      {GammaFactorRole, "gammaFactor"},
      {UvMinXRole,      "uvMinX"},
      {UvMinYRole,      "uvMinY"},
      {UvMaxXRole,      "uvMaxX"},
      {UvMaxYRole,      "uvMaxY"},
      {DevicesRole,     "devices"},
      {LabelRole,       "label"}
    };
  }


  QVariantMap ChannelListModel::get(int row) const
  {
    QVariantMap map;

    if(row < 0 || row >= static_cast<int>(m_entries.size())){
      return map;
    }

    const Entry& entry = m_entries.at(row);
    map["channelId"]   = entry.channelId;
    map["active"]      = entry.active;
    map["gammaFactor"] = entry.gammaFactor;
    map["uvMinX"]      = entry.uvMinX;
    map["uvMinY"]      = entry.uvMinY;
    map["uvMaxX"]      = entry.uvMaxX;
    map["uvMaxY"]      = entry.uvMaxY;
    map["devices"]     = entry.devices;
    map["label"]       = entry.label;

    return map;
  }


  void ChannelListModel::reload()
  {
    beginResetModel();

    m_entries.clear();

    if(m_core && m_core->running()){
      const Channels& channels = m_core->channels();

      std::vector<uint8_t> ids;
      ids.reserve(channels.size());
      for(const auto& [channelId, channel] : channels){
        Q_UNUSED(channel);
        ids.push_back(channelId);
      }
      std::sort(ids.begin(), ids.end());

      for(uint8_t channelId : ids){
        const Channel& channel = channels.at(channelId);
        const UVs& uvs = channel.uvs();

        Entry entry;
        entry.channelId = channelId;
        entry.active = channel.state() == Channel::State::Active;
        entry.gammaFactor = channel.gammaFactor();
        entry.uvMinX = uvs.min.x;
        entry.uvMinY = uvs.min.y;
        entry.uvMaxX = uvs.max.x;
        entry.uvMaxY = uvs.max.y;
        entry.label = QString("Channel %1").arg(channelId + 1);

        for(const Device& device : channel.devices()){
          entry.devices << QString::fromStdString(device.name);
        }

        m_entries.append(entry);
      }
    }

    endResetModel();
  }
}
