/*
 * Project: Crankshaft
 * This file is part of Crankshaft project.
 * Copyright (C) 2025 OpenCarDev Team
 *
 *  Crankshaft is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Crankshaft is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Crankshaft. If not, see <http://www.gnu.org/licenses/>.
*/

#include <catch2/catch_all.hpp>

#include <QCoreApplication>
#include <QSignalSpy>
#include <QThread>
#include <QVariantMap>

#include "EventBus.h"

TEST_CASE("EventBus singleton", "[eventbus]") {
  EventBus& bus1 = EventBus::instance();
  EventBus& bus2 = EventBus::instance();

  REQUIRE(&bus1 == &bus2);
}

TEST_CASE("EventBus publish and subscribe", "[eventbus]") {
  int argc = 0;
  char* argv[] = {nullptr};
  QCoreApplication app(argc, argv);

  EventBus& bus = EventBus::instance();
  QSignalSpy spy(&bus, &EventBus::messagePublished);

  QVariantMap payload;
  payload["key"] = "value";
  payload["number"] = 42;

  bus.publish("test/topic", payload);

  REQUIRE(spy.count() == 1);

  QList<QVariant> arguments = spy.takeFirst();
  REQUIRE(arguments.at(0).toString() == "test/topic");

  QVariantMap receivedPayload = arguments.at(1).toMap();
  REQUIRE(receivedPayload["key"].toString() == "value");
  REQUIRE(receivedPayload["number"].toInt() == 42);
}

TEST_CASE("EventBus multiple subscriptions", "[eventbus]") {
  int argc = 0;
  char* argv[] = {nullptr};
  QCoreApplication app(argc, argv);

  EventBus& bus = EventBus::instance();
  QSignalSpy spy(&bus, &EventBus::messagePublished);

  bus.publish("topic1", {{"data", "first"}});
  bus.publish("topic2", {{"data", "second"}});
  bus.publish("topic3", {{"data", "third"}});

  REQUIRE(spy.count() == 3);
}

TEST_CASE("EventBus thread safety", "[eventbus]") {
  int argc = 0;
  char* argv[] = {nullptr};
  QCoreApplication app(argc, argv);

  EventBus& bus = EventBus::instance();
  QSignalSpy spy(&bus, &EventBus::messagePublished);

  const int numThreads = 10;
  const int messagesPerThread = 100;

  QList<QThread*> threads;

  for (int i = 0; i < numThreads; ++i) {
    QThread* thread = QThread::create([&bus, i, messagesPerThread]() {
      for (int j = 0; j < messagesPerThread; ++j) {
        bus.publish(QString("thread_%1/message_%2").arg(i).arg(j), {{"thread", i}, {"message", j}});
      }
    });
    threads.append(thread);
    thread->start();
  }

  for (QThread* thread : threads) {
    thread->wait();
    delete thread;
  }

  // Process events to ensure signals are delivered
  QCoreApplication::processEvents();

  REQUIRE(spy.count() == numThreads * messagesPerThread);
}
