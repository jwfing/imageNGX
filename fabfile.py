import os

from datetime import datetime
from fabric.api import run, sudo, env, cd, local, prefix, put, lcd
from fabric.contrib.files import exists
from fabric.contrib.project import rsync_project

user = 'deploy'
dist = 'debian'
src_dir = '~/imageNGX'
output_dir = src_dir + '/webservice/output'
service_dir = '/var/backends/lean-image'
logroot_dir = '/var/logs'

def set_user_dir():
    global dist, user, service_dir, logroot_dir
    issue = run('cat /etc/issue').lower()
    if 'centos' in issue:
        dist = 'centos'
    elif 'ubuntu' in issue:
        dist = 'ubuntu'
        user = 'ubuntu'
        service_dir = '/mnt/leancloud/lean-image'
        logroot_dir = '/mnt/leancloud/logs'

def compile_files(target):
    # make the src code
    run("cd %s && git checkout master && git stash && git pull" % src_dir)
    run("cd %s && make clean && make" % src_dir)
    # prepare conf
    if target != 'prod':
        run("cp -v %s/conf/image_%s.conf %s/conf/image.conf" % (src_dir, target, src_dir))

def prepare_remote_dirs():
    set_user_dir()
    bin_dir = service_dir + '/bin'
    if not exists(bin_dir):
        sudo('mkdir -p %s' % bin_dir)
        sudo('chown %s %s' % (user, bin_dir))

    scripts_dir = service_dir + '/scripts/'
    if not exists(scripts_dir):
        sudo('mkdir -p %s' % scripts_dir)
        sudo('chown %s %s' % (user, scripts_dir))

    logs_dir = logroot_dir + '/lean-image'
    if not exists(logs_dir):
        sudo('mkdir -p %s' % logs_dir)
        sudo('chown %s %s' % (user, logs_dir))
    
def start_on_boot(name, dist):
    if dist == 'debian':
        sudo('update-rc.d %s defaults' % name)
    elif dist == 'ubuntu':
        sudo('update-rc.d %s defaults' % name)
    elif dist == 'centos':
        sudo('/sbin/chkconfig --level 3 %s on' % name)
    else:
        raise ValueError('dist can only take debian, centos')

def push_files():
    set_user_dir()
    if exists('/etc/init.d/lean-image'):
        sudo('/etc/init.d/lean-image stop')        
    run('cp -rvf %s/* %s/bin/' % (output_dir, service_dir))
    run('cp -rv %s/scripts/* %s/scripts/' % (src_dir, service_dir))

    # push init.d file and start service
    sudo('cp -fv %s/deploy/init/lean-image /etc/init.d/lean-image' % src_dir)
    sudo('/etc/init.d/lean-image stop')
    sudo('/etc/init.d/lean-image start')
    start_on_boot('lean-image', dist)

    # push nginx files
    #sudo('cp -v %s/deploy/nginx/image-service /etc/nginx/sites-available/' % src_dir)
    sudo('cp -v %s/deploy/nginx/cached.conf /etc/nginx/conf.d/' %src_dir)
    #sudo('/etc/init.d/nginx restart')

def deploy(target='prod'):
    compile_files(target)
    prepare_remote_dirs()
    push_files()
