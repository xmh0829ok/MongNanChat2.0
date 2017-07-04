#-*- coding: utf-8 -*-

import tornado.web
import torndb

class BaseHandler(tornado.web.RequestHandler):
    def get_current_user(self):
        return self.get_secure_cookie("username")

class IndexHandler(BaseHandler):

    users = set()

    @tornado.web.authenticated
    def get(self):
        #findfriend = 'select friendname from friends where username="%s"' % self.current_user

        self.render('index.html', username=self.current_user)
        userip = self.request.remote_ip
        username = self.current_user
        userport = 14514

        # 在线用户列表
        online = 'insert into entries(user_id, ip_address, port) values ("%s","%s","%s")' % (username, userip, userport)
        self.application.db.execute(online)
        sel = self.application.db.get('select ip_address from entries where user_id="%s"' % username)
        print sel['ip_address'] + type(sel['ip_address'])
        self.users.add(self.current_user)

        #拿到用户列表后 已知好友user_id
        #sel = self.application.db.get('select ip_address from entries where user_id="%s"' % username)
        #sel['ip_address']为ip地址， sel['port']为端口号
