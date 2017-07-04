#-*- coding: utf-8 -*-

import tornado.web
import threading
from socket import *

class BaseHandler(tornado.web.RequestHandler):
    def get_current_user(self):
        return self.get_secure_cookie("username")

class IndexHandler(BaseHandler):

    users = set()
    friends = []

    def socket_build(self, userip, userport):
        # 打开半连接
        ServerSock = socket(AF_INET, SOCK_STREAM)
        ADDR = (userip, userport)
        ServerSock.bind(ADDR)
        ServerSock.listen(5)

        def response(ClientSock, addr):
            pass

        while True:
            ClientSock, addr = ServerSock.accept()
            client_accept = threading.Thread(target=response, args=(ClientSock, addr))
            client_accept.start()

    @tornado.web.authenticated
    def get(self):
        userip = str(self.request.remote_ip)
        username = self.current_user
        userport = 14514

        friends_dict = self.application.db.query('select friendname from friends where username="%s"' % username)
        for f in friends_dict:
            self.friends.append(str(f['friendname']))

        self.render('index.html', username=self.current_user, friends=self.friends)

        # 在线用户列表
        online = 'insert into entries(user_id, ip_address, port) values ("%s","%s","%s")' % (username, userip, userport)
        self.application.db.execute(online)
        self.users.add(self.current_user)

        socket_thread = threading.Thread(target=self.socket_build, args=(userip, userport))
        socket_thread.start()
