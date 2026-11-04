#pragma once

#include <QAbstractListModel>
#include <QHash>
#include <QVector>
#include <QString>

namespace Huenicorn
{
  class HuenicornCore;

  /**
   * @brief List model exposing the available Hue entertainment configurations to the Qt Quick GUI
   * \n   */
  class EntertainmentConfigurationModel : public QAbstractListModel
  {
    Q_OBJECT

  public:
    // Type definitions
    enum Roles
    {
      IdRole = Qt::UserRole + 1,
      NameRole,
      CurrentRole
    };

    /**
     * @brief EntertainmentConfigurationModel constructor
     * 
     * @param core Pointer to Huenicorn core
     * @param parent Parent QObject
     */
    explicit EntertainmentConfigurationModel(HuenicornCore* core, QObject* parent = nullptr);


    // Overrides
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;


  public slots:
    /**
     * @brief Reloads the model content from the core entertainment configurations
     * 
     */
    void reload();


  private:
    // Type definitions
    struct Entry
    {
      QString id;
      QString name;
      bool current{false};
    };

    // Attributes
    HuenicornCore* m_core;
    QVector<Entry> m_entries;
  };
}
