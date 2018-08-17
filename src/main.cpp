/*
 * Bigeye Lite - Accessorial Tool for Daily Test
 *
 * Copyright (c) 2018, longfeng.xiao <xlongfeng@126.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include "bigeyelite.h"
#include "logmodel.h"

static QObject *bigeyelite_singletontype_provider(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)

    BigeyeLite *singleton = BigeyeLite::instance();
    return singleton;
}

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QGuiApplication app(argc, argv);

    Q_INIT_RESOURCE(qml);

    qmlRegisterSingletonType<BigeyeLite>("Backend", 1, 0, "BigeyeLite", bigeyelite_singletontype_provider);
    qmlRegisterType<LogModel>("Backend", 1, 0, "LogModel");

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
