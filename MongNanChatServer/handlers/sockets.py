<<<<<<< HEAD
#-*- coding :utf-8 -*-

import logging
import tornado.escape
import tornado.websocket

class SocketHandler(tornado.websocket.WebSocketHandler):
    users = set()
    cache = []
    cache_size = 200

    def check_origin(self, origin):
        return True

    def open(self):
        print 'new users joined'
        SocketHandler.users.add(self)

    def on_close(self):
        print 'a users left'
        SocketHandler.users.remove(self)

    @classmethod
    def update_cache(cls, chat):
        cls.cache.append(chat)
        if len(cls.cache) > cls.cache_size:
            cls.cache = cls.cache[-cls.cache_size:]

    @classmethod
    def send_updates(cls, chat):
        logging.info('sending message to %d users', len(cls.users))
        for user in cls.users:
            try:
                user.write_message(chat)
            except:
                logging.error("Error sending message", exc_info=True)

    def on_message(self, message):
        logging.info("got message %r", message)
        SocketHandler.send_updates(message)
=======
#-*- coding: utf-8 -*-
>>>>>>> d0645e0a89f1e7adca668b38b0434eada16fc3b5
