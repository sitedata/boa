#!/bin/bash

PATH=/usr/local/bin:/usr/local/sbin:/opt/local/bin:/usr/bin:/usr/sbin:/bin:/sbin
SHELL=/bin/bash
pthVhstd="/var/aegir/config/server_master/nginx/vhost.d"

if [ -e "/root/.proxy.cnf" ]; then
  exit 0
fi

hold() {
  service nginx stop &> /dev/null
  killall -9 nginx &> /dev/null
  sleep 1
  killall -9 nginx &> /dev/null
  _PHP_V="81 80 74 73 72 71 70 56 55 54 53"
  for e in ${_PHP_V}; do
    if [ -e "/etc/init.d/php${e}-fpm" ]; then
      service php${e}-fpm stop &> /dev/null
    fi
  done
  killall -9 php-fpm php-cgi &> /dev/null
  echo "$(date 2>&1)" >> /var/xdrago/log/second.hold.log
  ### echo "load is ${_O_LOAD}:${_F_LOAD} while
  ### maxload is ${_O_LOAD_MAX}:${_F_LOAD_MAX}"
}

terminate() {
  if [ ! -e "/var/run/boa_run.pid" ]; then
    killall -9 php drush.php wget curl &> /dev/null
    echo "$(date 2>&1)" >> /var/xdrago/log/second.terminate.log
  fi
}

nginx_high_load_on() {
  mv -f /data/conf/nginx_high_load_off.conf /data/conf/nginx_high_load.conf
  service nginx reload &> /dev/null
}

nginx_high_load_off() {
  mv -f /data/conf/nginx_high_load.conf /data/conf/nginx_high_load_off.conf
  service nginx reload &> /dev/null
}

check_vhost_health() {
  if [ -e "$1"* ]; then
    echo vhost $1 exists
    vhostPlaceTest=$(grep "### access" $1* 2>&1)
    vhostAllowTest=$(grep "allow .*;" $1* 2>&1)
    vhostDenyTest=$(grep "deny .*;" $1* 2>&1)
    if [[ "${vhostPlaceTest}" =~ "access" ]] \
      && [[ "${vhostDenyTest}" =~ "deny" ]]; then
      if [[ "${vhostAllowTest}" =~ "allow" ]]; then
        vhostHealthTest=YES
      else
        vhostHealthTest=YES
      fi
    else
      vhostHealthTest=NO
      sed -i "s/### access .*//g; \
        s/allow .*;//g; \
        s/deny .*;//g; \
        s/ *$//g; /^$/d" $1* &> /dev/null
      wait
      sed -i "s/limit_conn .*/limit_conn                   limreq 555;\n  \
        ### access none\n  deny                         all;/g" $1* &> /dev/null
      wait
    fi
  else
    echo vhost $1 does not exist
  fi
}

update_ip_auth_access() {
  touch /var/run/.auth.IP.list.pid
  if [ -e "/var/backups/.auth.IP.list.tmp" ]; then
    if [ -e "${pthVhstd}/adminer."* ]; then
      sed -i "s/### access .*//g; \
        s/allow .*;//g; \
        s/deny .*;//g; \
        s/ *$//g; /^$/d" \
        ${pthVhstd}/adminer.* &> /dev/null
      wait
      sed -i "s/limit_conn .*/limit_conn                   limreq 555;\n  \
        ### access update/g" \
        ${pthVhstd}/adminer.* &> /dev/null
      wait
    fi
    if [ -e "${pthVhstd}/chive."* ]; then
      sed -i "s/### access .*//g; \
        s/allow .*;//g; \
        s/deny .*;//g; \
        s/ *$//g; /^$/d" \
        ${pthVhstd}/chive.* &> /dev/null
      wait
      sed -i "s/limit_conn .*/limit_conn                   limreq 555;\n  \
        ### access update/g" \
        ${pthVhstd}/chive.* &> /dev/null
      wait
    fi
    if [ -e "${pthVhstd}/cgp."* ]; then
      sed -i "s/### access .*//g; \
        s/allow .*;//g; \
        s/deny .*;//g; \
        s/ *$//g; /^$/d" \
        ${pthVhstd}/cgp.* &> /dev/null
      wait
      sed -i "s/limit_conn .*/limit_conn                   limreq 555;\n  \
        ### access update/g" \
        ${pthVhstd}/cgp.* &> /dev/null
      wait
    fi
    if [ -e "${pthVhstd}/sqlbuddy."* ]; then
      sed -i "s/### access .*//g; \
        s/allow .*;//g; \
        s/deny .*;//g; \
        s/ *$//g; /^$/d" \
        ${pthVhstd}/sqlbuddy.* &> /dev/null
      wait
      sed -i "s/limit_conn .*/limit_conn                   limreq 555;\n  \
        ### access update/g" \
        ${pthVhstd}/sqlbuddy.* &> /dev/null
      wait
    fi
    sleep 1
    sed -i '/  ### access .*/ {r /var/backups/.auth.IP.list.tmp
d;};' ${pthVhstd}/adminer.* &> /dev/null
    wait
    sed -i '/  ### access .*/ {r /var/backups/.auth.IP.list.tmp
d;};' ${pthVhstd}/chive.* &> /dev/null
    wait
    sed -i '/  ### access .*/ {r /var/backups/.auth.IP.list.tmp
d;};' ${pthVhstd}/cgp.* &> /dev/null
    wait
    sed -i '/  ### access .*/ {r /var/backups/.auth.IP.list.tmp
d;};' ${pthVhstd}/sqlbuddy.* &> /dev/null
    wait
    mv -f ${pthVhstd}/sed* /var/backups/
    check_vhost_health "${pthVhstd}/adminer."
    check_vhost_health "${pthVhstd}/chive."
    check_vhost_health "${pthVhstd}/cgp."
    check_vhost_health "${pthVhstd}/sqlbuddy."
    ngxTest=$(service nginx configtest 2>&1)
    if [[ "${ngxTest}" =~ "successful" ]]; then
      service nginx reload &> /dev/null
    else
      service nginx reload &> /var/backups/.auth.IP.list.ops
      sed -i "s/allow .*;//g; s/ *$//g; /^$/d" \
        ${pthVhstd}/adminer.*    &> /dev/null
      wait
      sed -i "s/allow .*;//g; s/ *$//g; /^$/d" \
        ${pthVhstd}/chive.*    &> /dev/null
      wait
      sed -i "s/allow .*;//g; s/ *$//g; /^$/d" \
        ${pthVhstd}/cgp.*      &> /dev/null
      wait
      sed -i "s/allow .*;//g; s/ *$//g; /^$/d" \
        ${pthVhstd}/sqlbuddy.* &> /dev/null
      wait
      check_vhost_health "${pthVhstd}/adminer."
      check_vhost_health "${pthVhstd}/chive."
      check_vhost_health "${pthVhstd}/cgp."
      check_vhost_health "${pthVhstd}/sqlbuddy."
      service nginx reload &> /dev/null
    fi
  fi
  rm -f /var/backups/.auth.IP.list
  for _IP in `who --ips \
    | sed 's/.*tty.*//g; s/.*root.*hvc.*//g; s/^[0-9]+$//g' \
    | cut -d: -f2 \
    | cut -d' ' -f2 \
    | sed 's/.*\/.*:S.*//g; s/:S.*//g; s/(//g' \
    | tr -d "\s" \
    | sort \
    | uniq`;do _IP=${_IP//[^0-9.]/}; if [[ "${_IP}" =~ "." ]]; then echo "  allow                        ${_IP};" \
      >> /var/backups/.auth.IP.list;fi;done
  if [ -e "/root/.ip.protected.vhost.whitelist.cnf" ]; then
    for _IP in `cat /root/.ip.protected.vhost.whitelist.cnf \
      | sort \
      | uniq`;do _IP=${_IP//[^0-9.]/}; if [[ "${_IP}" =~ "." ]]; then echo "  allow                        ${_IP};" \
        >> /var/backups/.auth.IP.list;fi;done
  fi
  sed -i "s/\.;/;/g; s/allow                        ;//g; s/ *$//g; /^$/d" \
    /var/backups/.auth.IP.list &> /dev/null
  wait
  if [ -e "/var/backups/.auth.IP.list" ]; then
    allowTestList=$(grep allow /var/backups/.auth.IP.list 2>&1)
  fi
  if [[ "${allowTestList}" =~ "allow" ]]; then
    echo "  deny                         all;" >> /var/backups/.auth.IP.list
    echo "  ### access live"                   >> /var/backups/.auth.IP.list
  else
    echo "  deny                         all;" >  /var/backups/.auth.IP.list
    echo "  ### access none"                   >> /var/backups/.auth.IP.list
  fi
  sleep 1
  rm -f /var/run/.auth.IP.list.pid
}

manage_ip_auth_access() {
  for _IP in `who --ips \
    | sed 's/.*tty.*//g; s/.*root.*hvc.*//g; s/^[0-9]+$//g' \
    | cut -d: -f2 \
    | cut -d' ' -f2 \
    | sed 's/.*\/.*:S.*//g; s/:S.*//g; s/(//g' \
    | tr -d "\s" \
    | sort \
    | uniq`;do _IP=${_IP//[^0-9.]/}; if [[ "${_IP}" =~ "." ]]; then echo "  allow                        ${_IP};" \
      >> /var/backups/.auth.IP.list.tmp;fi;done
  if [ -e "/root/.ip.protected.vhost.whitelist.cnf" ]; then
    for _IP in `cat /root/.ip.protected.vhost.whitelist.cnf \
      | sort \
      | uniq`;do _IP=${_IP//[^0-9.]/}; if [[ "${_IP}" =~ "." ]]; then echo "  allow                        ${_IP};" \
        >> /var/backups/.auth.IP.list.tmp;fi;done
  fi
  sed -i "s/\.;/;/g; s/allow                        ;//g; s/ *$//g; /^$/d" \
    /var/backups/.auth.IP.list.tmp &> /dev/null
  wait
  if [ -e "/var/backups/.auth.IP.list.tmp" ]; then
    allowTestTmp=$(grep allow /var/backups/.auth.IP.list.tmp 2>&1)
  fi
  if [[ "${allowTestTmp}" =~ "allow" ]]; then
    echo "  deny                         all;" >> /var/backups/.auth.IP.list.tmp
    echo "  ### access live"                   >> /var/backups/.auth.IP.list.tmp
  else
    echo "  deny                         all;" >  /var/backups/.auth.IP.list.tmp
    echo "  ### access none"                   >> /var/backups/.auth.IP.list.tmp
  fi
  if [ ! -e "/var/run/.auth.IP.list.pid" ]; then
    if [ ! -e "/var/backups/.auth.IP.list" ]; then
      update_ip_auth_access
    else
      if [ -e "/var/backups/.auth.IP.list.tmp" ]; then
        diffTestIf=$(diff -w -B /var/backups/.auth.IP.list.tmp \
          /var/backups/.auth.IP.list 2>&1)
        if [ ! -z "${diffTestIf}" ]; then
          update_ip_auth_access
        fi
      fi
    fi
  fi
  if [ -L "/var/backups/.vhost.d.mstr" ]; then
    if [ ! -d "/var/backups/.vhost.d.wbhd" ]; then
      mkdir -p /var/backups/.vhost.d.wbhd
      chmod 700 /var/backups/.vhost.d.wbhd
      cp -af /var/backups/.vhost.d.mstr/* /var/backups/.vhost.d.wbhd/
    fi
    diffClstrTest=$(diff -w -B /var/backups/.vhost.d.wbhd \
      /var/backups/.vhost.d.mstr 2>&1)
    if [ ! -z "${diffClstrTest}" ]; then
      service nginx reload &> /dev/null
      rm -rf /var/backups/.vhost.d.wbhd
      mkdir -p /var/backups/.vhost.d.wbhd
      chmod 700 /var/backups/.vhost.d.wbhd
      cp -af /var/backups/.vhost.d.mstr/* /var/backups/.vhost.d.wbhd/
    fi
  fi
  if [[ "${allowTestTmp}" =~ "allow" ]]; then
    vhostStatusAdminer=TRUE
    vhostStatusChive=TRUE
    vhostStatusCgp=TRUE
    vhostStatusBuddy=TRUE
    if [ -e "${pthVhstd}/adminer."* ]; then
      vhostStatusAdminer=FALSE
      vhostTestAdminer=$(grep allow ${pthVhstd}/adminer.* 2>&1)
      if [[ "${vhostTestAdminer}" =~ "allow" ]]; then
        vhostStatusAdminer=TRUE
      fi
    fi
    if [ -e "${pthVhstd}/chive."* ]; then
      vhostStatusChive=FALSE
      vhostTestChive=$(grep allow ${pthVhstd}/chive.* 2>&1)
      if [[ "${vhostTestChive}" =~ "allow" ]]; then
        vhostStatusChive=TRUE
      fi
    fi
    if [ -e "${pthVhstd}/cgp."* ]; then
      vhostStatusCgp=FALSE
      vhostTestCgp=$(grep allow ${pthVhstd}/cgp.* 2>&1)
      if [[ "${vhostTestCgp}" =~ "allow" ]]; then
        vhostStatusCgp=TRUE
      fi
    fi
    if [ -e "${pthVhstd}/sqlbuddy."* ]; then
      vhostStatusBuddy=FALSE
      vhostTestBuddy=$(grep allow ${pthVhstd}/sqlbuddy.* 2>&1)
      if [[ "${vhostTestBuddy}" =~ "allow" ]]; then
        vhostStatusBuddy=TRUE
      fi
    fi
    if [ "${vhostStatusAdminer}" = "FALSE" ] \
      || [ "${vhostStatusChive}" = "FALSE" ] \
      || [ "${vhostStatusCgp}" = "FALSE" ] \
      || [ "${vhostStatusBuddy}" = "FALSE" ]; then
      update_ip_auth_access
    fi
  fi
  rm -f /var/backups/.auth.IP.list.tmp
}

proc_control() {
  if [ "${_O_LOAD}" -ge "${_O_LOAD_MAX}" ]; then
    hold
  elif [ "${_F_LOAD}" -ge "${_F_LOAD_MAX}" ]; then
    hold
  else
    echo "load is ${_O_LOAD}:${_F_LOAD} while \
      maxload is ${_O_LOAD_MAX}:${_F_LOAD_MAX}"
    echo ...OK now running proc_num_ctrl...
    renice ${_B_NICE} -p $$ &> /dev/null
    perl /var/xdrago/proc_num_ctrl.cgi
    touch /var/xdrago/log/proc_num_ctrl.done
    echo CTL done
  fi
}

load_control() {
  _O_LOAD=$(awk '{print $1*100}' /proc/loadavg 2>&1)
  echo _O_LOAD is ${_O_LOAD}
  _O_LOAD=$(( _O_LOAD / _CPU_NR ))
  echo _O_LOAD per CPU is ${_O_LOAD}

  _F_LOAD=$(awk '{print $2*100}' /proc/loadavg 2>&1)
  echo _F_LOAD is ${_F_LOAD}
  _F_LOAD=$(( _F_LOAD / _CPU_NR ))
  echo _F_LOAD per CPU is ${_F_LOAD}

  _O_LOAD_SPR=$(( 100 * _CPU_SPIDER_RATIO ))
  echo _O_LOAD_SPR is ${_O_LOAD_SPR}

  _F_LOAD_SPR=$(( _O_LOAD_SPR / 9 ))
  _F_LOAD_SPR=$(( _F_LOAD_SPR * 7 ))
  echo _F_LOAD_SPR is ${_F_LOAD_SPR}

  _O_LOAD_MAX=$(( 100 * _CPU_MAX_RATIO ))
  echo _O_LOAD_MAX is ${_O_LOAD_MAX}

  _F_LOAD_MAX=$(( _O_LOAD_MAX / 9 ))
  _F_LOAD_MAX=$(( _F_LOAD_MAX * 7 ))
  echo _F_LOAD_MAX is ${_F_LOAD_MAX}

  _O_LOAD_CRT=$(( _CPU_CRIT_RATIO * 100 ))
  echo _O_LOAD_CRT is ${_O_LOAD_CRT}

  _F_LOAD_CRT=$(( _O_LOAD_CRT / 9 ))
  _F_LOAD_CRT=$(( _F_LOAD_CRT * 7 ))
  echo _F_LOAD_CRT is ${_F_LOAD_CRT}

  if [ "${_O_LOAD}" -ge "${_O_LOAD_SPR}" ] \
    && [ "${_O_LOAD}" -lt "${_O_LOAD_MAX}" ] \
    && [ -e "/data/conf/nginx_high_load_off.conf" ]; then
    nginx_high_load_on
  elif [ "${_F_LOAD}" -ge "${_F_LOAD_SPR}" ] \
    && [ "${_F_LOAD}" -lt "${_F_LOAD_MAX}" ] \
    && [ -e "/data/conf/nginx_high_load_off.conf" ]; then
    nginx_high_load_on
  elif [ "${_O_LOAD}" -lt "${_O_LOAD_SPR}" ] \
    && [ "${_F_LOAD}" -lt "${_F_LOAD_SPR}" ] \
    && [ -e "/data/conf/nginx_high_load.conf" ]; then
    nginx_high_load_off
  fi

  if [ "${_O_LOAD}" -ge "${_O_LOAD_CRT}" ]; then
    terminate
  elif [ "${_F_LOAD}" -ge "${_F_LOAD_CRT}" ]; then
    terminate
  fi

  proc_control
}

check_fastcgi_temp() {
  _FASTCGI_SIZE_TEST=$(du -s -h /usr/fastcgi_temp/*/*/* | grep G 2> /dev/null)
  if [[ "${_FASTCGI_SIZE_TEST}" =~ "G" ]]; then
    echo "fastcgi_temp too big"
    echo "$(date 2>&1) fastcgi_temp too big, cleanup forced START" >> \
      /var/xdrago/log/giant.fastcgi.incident.log
    echo "$(date 2>&1) ${_FASTCGI_SIZE_TEST}" >> \
      /var/xdrago/log/giant.fastcgi.incident.log
    rm -f /usr/fastcgi_temp/*/*/*
    hold
    echo "$(date 2>&1) fastcgi_temp too big, cleanup forced END" >> \
      /var/xdrago/log/giant.fastcgi.incident.log
  fi
}

count_cpu() {
  _CPU_INFO=$(grep -c processor /proc/cpuinfo 2>&1)
  _CPU_INFO=${_CPU_INFO//[^0-9]/}
  _NPROC_TEST=$(which nproc 2>&1)
  if [ -z "${_NPROC_TEST}" ]; then
    _CPU_NR="${_CPU_INFO}"
  else
    _CPU_NR=$(nproc 2>&1)
  fi
  _CPU_NR=${_CPU_NR//[^0-9]/}
  if [ ! -z "${_CPU_NR}" ] \
    && [ ! -z "${_CPU_INFO}" ] \
    && [ "${_CPU_NR}" -gt "${_CPU_INFO}" ] \
    && [ "${_CPU_INFO}" -gt "0" ]; then
    _CPU_NR="${_CPU_INFO}"
  fi
  if [ -z "${_CPU_NR}" ] || [ "${_CPU_NR}" -lt "1" ]; then
    _CPU_NR=1
  fi
}

if [ -e "/root/.barracuda.cnf" ]; then
  source /root/.barracuda.cnf
  _CPU_SPIDER_RATIO=${_CPU_SPIDER_RATIO//[^0-9]/}
  _CPU_MAX_RATIO=${_CPU_MAX_RATIO//[^0-9]/}
  _CPU_CRIT_RATIO=${_CPU_CRIT_RATIO//[^0-9]/}
  _B_NICE=${_B_NICE//[^0-9]/}
fi

if [ -z "${_CPU_SPIDER_RATIO}" ]; then
  _CPU_SPIDER_RATIO=3
fi
if [ -z "${_CPU_MAX_RATIO}" ]; then
  _CPU_MAX_RATIO=6
fi
if [ -z "${_CPU_CRIT_RATIO}" ]; then
  _CPU_CRIT_RATIO=9
fi
if [ -z "${_B_NICE}" ]; then
  _B_NICE=10
fi

if [ ! -e "/var/tmp/fpm" ]; then
  mkdir -p /var/tmp/fpm
  chmod 777 /var/tmp/fpm
fi

check_fastcgi_temp
count_cpu
load_control
sleep 3
load_control
sleep 3
load_control
sleep 3
load_control
sleep 3
load_control
sleep 3
load_control
sleep 3
load_control
sleep 3
load_control
sleep 3
load_control
sleep 3
load_control
sleep 3
load_control
sleep 3
load_control
sleep 3
load_control
sleep 3
load_control
sleep 3
load_control
sleep 3
load_control
sleep 3
load_control
sleep 3
load_control
sleep 3
load_control
sleep 3
load_control
manage_ip_auth_access
echo Done !
exit 0
###EOF2023###
