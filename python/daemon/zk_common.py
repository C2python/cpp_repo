# -*- coding: utf-8 -*-

import traceback
import random
import time
from kazoo.client import KazooClient
from kazoo.client import KazooState
import logging

LOG = logging.getLogger(__name__)

class ZKBase(object):
    """
    scheduler需要定义以下函数：
    - callback(KazooState.*)： 处理CONN: LOST,SUSPENDED
    - 
    """

    def __init__(self,conf,scheduler):
        self.conf = conf
        self.state = "STOPPED" 
        self.zk = KazooClient(self.conf.processor.zk_url, timeout=self.conf.processor.timeout)
        try:
            self.zk.start()
        except Exception:
            LOG.error("Start zk error: %s" % traceback.format_exc(limit=2))
        self.zk.add_listener(self.listen)
        self.scheduler = scheduler
        self.state = "RUNNING" # STOPPED
        self.state_changes = False
        
    def listen(self, state):
        if state == KazooState.LOST or state == KazooState.SUSPENDED:
            LOG.error("Session state change: %s" % state)
            if self.state == "STOPPED":
                return
            self.state_changes = True
            self.scheduler.callback(state)
        elif state == KazooState.CONNECTED:
            LOG.info("Connectied to ZK.")
        else:
            LOG.error("Session timeout. Cannot conect to ZK.")

    def reinit(self,state):
        while self.zk.state != KazooState.CONNECTED:
            LOG.info("Restart zk connection until connected.")
            try:
                self.zk.restart()
            except Exception:
                LOG.error("Reinit: %s" % traceback.format_exc(limit=1))
        self.state_changes = False

    def create(self,path, value='', acl=None, ephemeral=False, sequence=False, makepath=False):
        LOG.info("Create: path %s" % path)
        self.zk.create(path=path,
                        value=value.encode(),
                        ephemeral=ephemeral,
                        sequence=sequence,
                        makepath=makepath)

    def get(self,path,watch=None):
        self.zk.get(path,watch)

    def get_children(self,path,watch=None):
        return self.zk.get_children(path,watch=watch)

    def terminate(self):
        LOG.info("Terminate ZK connection.")
        self.state = "STOPPED"
        self.zk.stop()
        self.zk.close()

class HAMaster(object):

    def __init__(self,conf,zk,scheduler):
        self.conf = conf
        self.path = "/updns/master"
        self.is_leader = False
        self.zk = zk
        self.scheduler = scheduler
    
    @property
    def leader(self):
        return self.is_leader

    @leader.setter
    def leader(self,role):
        self.is_leader = role

    def create_instance(self,work_id):
        node = self.path + '/' + work_id + '-by-'
        LOG.info("Proccessor HA zk node path is: %s" % node)
        self.zk.create(path=node, value="", acl=None, ephemeral=True, sequence=True, makepath=True)

    def my_watch(self,event):
        if event.state == "CONNECTED" and event.type == "CREATED" or event.type == "DELETED" or event.type == "CHANGED" or event.type == "CHILD":
            LOG.info("Children stat changed. Reelect")
            self._elect()
        else:
            LOG.info("No monitor event.")

    def elect(self):
        LOG.info("Starting master election.")
        self.create_instance(self.scheduler.member_id)
        self._elect()

    def _elect(self):
        instance_list = self.zk.get_children(path=self.path, watch=self.my_watch)
        instance = max(instance_list).split("-by-")[0]
        if instance == self.scheduler.member_id:
            if not self.is_leader:
                LOG.info("Elected as master, work_id: %s." % self.scheduler.member_id)
                self.is_leader = True
            else:
                LOG.info("Elected as master as prevous, work_id: %s." % self.scheduler.member_id)
        else:
            LOG.info("Standby, work_id: %s." % self.scheduler.member_id)
            self.is_leader = False
        LOG.info("End master election")

    def reinit(self,state):
        self.is_leader = False
        self.elect()
        
    def terminate(self):
        LOG.info("Terminate HA. Nothing to do.")

'''
if __name__ == "__main__":
    zkcli = ZKBase()
'''