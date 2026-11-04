#pragma once

#include <QAbstractListModel>
#include <QHash>
#include <QVector>
#include <QStringList>
#include <QVariantMap>

namespace Huenicorn
{
  class HuenicornCore;

  /**
   * @brief List model exposing the Hue channels to the Qt Quick GUI
   * 
   */
  class ChannelListModel : public QAbstractListModel
  {
    Q_OBJECT

  public:
    // Type definitions
    enum Roles
    {
      ChannelIdRole = Qt::UserRole + 1,
      ActiveRole,
      GammaFactorRole,
      UvMinXRole,
      UvMinYRole,
      UvMaxXRole,
      UvMaxYRole,
      DevicesRole,
      LabelRole
    };

    /**
     * @brief ChannelListModel constructor
     * 
     * @param core Pointer to Huenicorn core
     * @param parent Parent QObject
     */
    explicit ChannelListModel(HuenicornCore* core, QObject* parent = nullptr);


    // Overrides
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;


    // Methods
    /**
     * @brief Returns the entry at given row as a map of role names to values
     * 
     * @param row Row index
     * @return QVariantMap Entry data
     */
    Q_INVOKABLE QVariantMap get(int row) const;


  public slots:
    /**
     * @brief Reloads the model content from the core channels
     * 
     */
    void reload();


  private:
    // Type definitions
    struct Entry
    {
      int channelId{-1};
      bool active{false};
      double gammaFactor{0.0};
      double uvMinX{0.0};
      double uvMinY{0.0};
      double uvMaxX{1.0};
      double uvMaxY{1.0};
      QStringList devices;
      QString label;
    };

    // Attributes
    HuenicornCore* m_core;
    QVector<Entry> m_entries;
  };
}
