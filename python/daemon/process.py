# -*- coding: utf-8 -*-

import socket
import threading
import time
import uuid
import sys

import cachetools.func
import cotyledon
from cotyledon import oslo_config_glue
#from oslo_config import cfg
#from oslo_log import log

import utils
import zk_common

import logging

logging.basicConfig(level="INFO",format='%(levelname)s: %(message)s',stream = sys.stderr)

LOG = logging.getLogger(__name__)

class Config(object):
    class Processor(object):
        def __init__(self):
            self.workers = 3
            self.zk_url = "10.10.2.5:2181"
            self.timeout = 10
            self.interval_delay = 8
    def __init__(self):
        self.processor = self.Processor()

class ProcessBase(cotyledon.Service):
    def __init__(self, worker_id, conf):
        super(ProcessBase, self).__init__(worker_id)
        self.conf = conf
        self.template_path = "/updns/templates"
        self.domain_path = "/updns/domains"
        self.startup_delay = self.worker_id = worker_id
        self.interval_delay = self.conf.processor.interval_delay
        self._wake_up = threading.Event()
        self._shutdown = threading.Event()
        self._shutdown_done = threading.Event()

    def callback(self,state):
        self.wakeup()

    def wakeup(self):
        self._wake_up.set()

    def _configure(self):
        self.member_id ="%s.%s.%s" % (socket.gethostname(),
                                        self.worker_id,
                                        # NOTE(jd) Still use a uuid here so we're
                                        # sure there's no conflict in case of
                                        # crash/restart
                                        str(uuid.uuid4()))
        
        self.drivers = utils.load_drivers()
        self.zkcli = zk_common.ZKBase(self.conf,self)
        self.ha = zk_common.HAMaster(self.conf,self.zkcli,self)

    def run(self):
        self._configure()
        # Delay startup so workers are jittered.
        self.ha.elect()
        time.sleep(self.startup_delay)
        while not self._shutdown.is_set():
            with utils.StopWatch() as timer:
                try:
                    if self.ha.leader:
                        self._run_job()
                    else:
                        LOG.info("Standby, go to sleep. worker_id: %s" % self.worker_id)
                except Exception:
                    LOG.error("Unexpected error during %s job",
                              self.name,
                              exc_info=True)
            self._wake_up.wait(max(0, self.interval_delay - timer.elapsed()))
            self._wake_up.clear()
            if self.zkcli.state_changes:
                self.zkcli.reinit(state)
                self.ha.reinit(state)
        self._shutdown_done.set()

    def is_shutdown(self):
        return self._shutdown.is_set()

    def terminate(self):
        self._shutdown.set()
        self.wakeup()
        LOG.info("Waiting ongoing domain processing to finish")
        self._shutdown_done.wait()
        self.close_services()

    def close_services(self):
        self.zkcli.terminate()
        self.ha.terminate()

    def _run_job(self):
        retry = 0
        while retry<2:
            print("Run Job: %s, work_id: %s" % (str(retry),self.worker_id))
            #print("HA Status: %s" % self.zkcli.zk.state)
            retry += 1
            time.sleep(0.1)

class UpdnsServiceManager(cotyledon.ServiceManager):
    def __init__(self, conf):
        super(UpdnsServiceManager, self).__init__()
        #oslo_config_glue.setup(self, conf)

        self.conf = conf
        self.processor_id = self.add(ProcessBase, args=(self.conf,),workers=conf.processor.workers)
        self.register_hooks(on_reload=self.on_reload)

    def on_reload(self):
        # NOTE(sileht): We do not implement reload() in Workers so all workers
        # will received SIGHUP and exit gracefully, then their will be
        # restarted with the new number of workers. This is important because
        # we use the number of worker to declare the capability in tooz and
        # to select the block of metrics to proceed.
        self.reconfigure(self.processor_id,
                         workers=self.conf.processor.workers)

def process(conf):
    UpdnsServiceManager(conf).run()

conf = Config()

if __name__ == "__main__":
    process(conf)