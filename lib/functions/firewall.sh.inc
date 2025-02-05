#
# Fix csf.uidignore file to whitelist important system uids when UID_INTERVAL != 0
fix_lfd_uidignore() {
  _THIS_FILE=/etc/csf/csf.uidignore
  if [ -e "${_THIS_FILE}" ]; then
    _CSF_UIDIGNORE_TEST=$(grep "pdnsd" ${_THIS_FILE} 2>&1)
    if [[ "$_CSF_UIDIGNORE_TEST" =~ "pdnsd" ]]; then
      _DO_NOTHING=YES
    else
      echo "#root"          >> /etc/csf/csf.uidignore
      echo `id -u root`     >> /etc/csf/csf.uidignore
      if [ -r "/etc/pdnsd.conf" ]; then
        echo "#pdnsd"       >> /etc/csf/csf.uidignore
        echo `id -u pdnsd`  >> /etc/csf/csf.uidignore
      fi
      echo "#postfix"       >> /etc/csf/csf.uidignore
      echo `id -u postfix`  >> /etc/csf/csf.uidignore
      echo "#www-data"      >> /etc/csf/csf.uidignore
      echo `id -u www-data` >> /etc/csf/csf.uidignore
    fi
    if [ -e "/usr/sbin/named" ]; then
      _CSF_UIDIGNORE_TEST=$(grep "bind" ${_THIS_FILE} 2>&1)
      if [[ "$_CSF_UIDIGNORE_TEST" =~ "bind" ]]; then
        _DO_NOTHING=YES
      else
        echo "#bind"        >> /etc/csf/csf.uidignore
        echo `id -u bind`   >> /etc/csf/csf.uidignore
      fi
    fi
    sed -i "/^$/d" ${_THIS_FILE} &> /dev/null
  fi
}

#
# Fix csf.fignore file to whitelist /tmp/drush_*
fix_lfd_whitelist() {
  _THIS_FILE=/etc/csf/csf.fignore
  if [ -e "${_THIS_FILE}" ]; then
    _CSF_WHITELIST_TEST=$(grep "jetty" ${_THIS_FILE} 2>&1)
    if [[ "$_CSF_WHITELIST_TEST" =~ "jetty" ]]; then
      _DO_NOTHING=YES
    else
      sed -i "s/.*\/tmp\/.*//g" ${_THIS_FILE} &> /dev/null
      wait
      sed -i "/^$/d"            ${_THIS_FILE} &> /dev/null
      wait
      echo "/tmp/drush_tmp.*"      >> ${_THIS_FILE}
      echo "/tmp/drush_make_tmp.*" >> ${_THIS_FILE}
      echo "/tmp/make_tmp.*"       >> ${_THIS_FILE}
      echo "/tmp/hsperfdata.*"     >> ${_THIS_FILE}
      echo "/tmp/jetty.*"          >> ${_THIS_FILE}
    fi
  fi
}

#
# install csf/lfd firewall
csf_install() {
  if [ "$_CSF_MODE" = "install" ]; then
    msg "INFO: Installing csf/lfd firewall..."
  else
    msg "INFO: Upgrading csf/lfd firewall..."
  fi
  cd /var/opt
  _IPSET_TEST=$(which ipset 2> /dev/null)
  if [ ! -x "${_IPSET_TEST}" ]; then
    apt-get clean -qq &> /dev/null
    rm -rf /var/lib/apt/lists/*
    apt-get update -qq &> /dev/null
    apt-get install ipset ${forCer} &> /dev/null
  fi
  if [ -x "/sbin/ipset" ] && [ ! -L "/usr/sbin/ipset" ]; then
    rm -f /usr/sbin/ipset
    ln -sf /sbin/ipset /usr/sbin/ipset
  fi
  rm -f ${pthLog}/lastFire
  ### from https://download.configserver.com/csf.tgz
  get_dev_src "csf-${_CSF_VRN}.tgz"
  cd csf
  mrun "sh install.sh" 2> /dev/null
  cd /var/opt
  if [[ "${_THISHOST}" =~ ".host8." ]] \
    || [[ "${_THISHOST}" =~ ".boa.io" ]] \
    || [[ "${_THISHOST}" =~ ".o8.io" ]] \
    || [[ "${_THISHOST}" =~ ".aegir.cc" ]] \
    || [ "${_VMFAMILY}" = "VS" ]; then
    _SSH_PORT=22
  fi
  _CSF_COMPATIBILITY_TEST=$(perl /etc/csf/csftest.pl 2>&1)
  if [[ "$_CSF_COMPATIBILITY_TEST" =~ "RESULT: csf should function" ]]; then
    _CSF_COMPATIBILITY=YES
  elif [[ "$_CSF_COMPATIBILITY_TEST" =~ "some features will not work" ]]; then
    _CSF_COMPATIBILITY=PARTIAL
    sed -i "s/^PORTFLOOD .*/PORTFLOOD = \"\"/g" /etc/csf/csf.conf &> /dev/null
    wait
    sed -i "s/^CONNLIMIT .*/CONNLIMIT = \"\"/g" /etc/csf/csf.conf &> /dev/null
    wait
    sed -i "s/^USE_CONNTRACK .*/USE_CONNTRACK = \"0\"/g" /etc/csf/csf.conf &> /dev/null
    wait
  elif [[ "$_CSF_COMPATIBILITY_TEST" =~ "FATAL" ]]; then
    _CSF_COMPATIBILITY=NO
  else
    _CSF_COMPATIBILITY=NO
  fi
  if [ "$_CSF_COMPATIBILITY" = "YES" ] \
    || [ "$_CSF_COMPATIBILITY" = "PARTIAL" ]; then
    if [ "$_CSF_COMPATIBILITY" = "PARTIAL" ]; then
      msg "TEST: csf/lfd firewall should mostly work on this system"
    else
      msg "TEST: csf/lfd firewall should work fine on this system"
    fi
    mv -f /etc/csf/csf.conf \
      /etc/csf/csf.conf-pre-${_X_VERSION}-${_NOW} &> /dev/null
    cp -af ${locCnf}/csf.conf /etc/csf/csf.conf
    sed -i "s/notify\@omega8.cc/${_MY_EMAIL}/g" /etc/csf/csf.conf &> /dev/null
    wait
    sed -i "s/TCP_IN = \"20,21,22,/TCP_IN = \"20,21,${_SSH_PORT},/g" \
      /etc/csf/csf.conf &> /dev/null
    wait
    sed -i "s/^CC_SRC .*/CC_SRC = \"2\"/g" /etc/csf/csf.conf &> /dev/null
    wait
    chmod 600 /etc/csf/csf.conf &> /dev/null
    if [ -e "/etc/ssh/sshd_config" ]; then
      sed -i "s/^Port.*/Port ${_SSH_PORT}/g"  /etc/ssh/sshd_config
      wait
      sed -i "s/^#Port.*/Port ${_SSH_PORT}/g" /etc/ssh/sshd_config
      wait
    fi
    if [ -e "/usr/etc/sshd_config" ]; then
      sed -i "s/^Port.*/Port ${_SSH_PORT}/g"  /usr/etc/sshd_config
      wait
      sed -i "s/^#Port.*/Port ${_SSH_PORT}/g" /usr/etc/sshd_config
      wait
      sed -i "s/^UsePrivilegeSeparation.*//g" /usr/etc/sshd_config
      wait
    fi
    mrun "service ssh restart" &> /dev/null
    if [ "$_CSF_MODE" = "install" ]; then
      msg "INFO: csf/lfd firewall installed"
    else
      msg "INFO: csf/lfd firewall upgrade completed"
    fi
    touch ${pthLog}/csf_${_X_VERSION}.log
  else
    msg "TEST: csf/lfd firewall can not be installed on this system"
  fi
}

install_upgrade_csf_lfd() {
  _IPSET_TEST=$(which ipset 2> /dev/null)
  if [ ! -x "${_IPSET_TEST}" ]; then
    apt-get clean -qq &> /dev/null
    rm -rf /var/lib/apt/lists/*
    apt-get update -qq &> /dev/null
    apt-get install ipset ${forCer} &> /dev/null
  fi
  if [ -x "/sbin/ipset" ] && [ ! -L "/usr/sbin/ipset" ]; then
    rm -f /usr/sbin/ipset
    ln -sf /sbin/ipset /usr/sbin/ipset
  fi
  if [ -e "/root/.mstr.clstr.cnf" ] \
    || [ -e "/root/.wbhd.clstr.cnf" ] \
    || [ -e "/root/.dbhd.clstr.cnf" ]; then
    if [ "${_USE_CSF_ON_CLSTR}" = "YES" ]; then
      _CSF_CLSTR=YES
    else
      _CSF_CLSTR=NO
    fi
  fi
  if [[ "${_XTRAS_LIST}" =~ "ALL" ]] \
    || [[ "${_XTRAS_LIST}" =~ "CSF" ]] \
    || [ "$_CSF_CLSTR" = "YES" ]; then
    if [ ! -e "/var/run/cloud_vhost.pid" ]; then
      if [ ! -e "/usr/sbin/csf" ]; then
        echo " "
        if prompt_yes_no "Do you want to install csf/lfd firewall?" ; then
          true
          _CSF_MODE=install
          csf_install
        else
          msg "INFO: csf/lfd firewall installation skipped"
        fi
      fi
    fi
  fi
  if [ -e "/usr/sbin/csf" ] \
    || [ -e "/usr/sbin/lfd" ] \
    || [ -e "/etc/cron.d/lfd" ]; then
    if [ "$_CSF_COMPATIBILITY" = "NO" ] \
      || [ "$_CSF_CLSTR" = "NO" ]; then
      _REMOVE_CSF=YES
    elif [ "${_VMFAMILY}" = "VS" ] \
      && [ ! -e "/boot/grub/grub.cfg" ] \
      && [ ! -e "/boot/grub/menu.lst" ]; then
      _REMOVE_CSF=YES
    fi
    if [ "$_REMOVE_CSF" = "YES" ]; then
      service lfd stop &> /dev/null
      sleep 3
      kill -9 $(ps aux | grep '[C]onfigServer' | awk '{print $2}') &> /dev/null
      killall sleep &> /dev/null
      rm -f /etc/csf/csf.error
      csf -x &> /dev/null
      update-rc.d -f csf remove &> /dev/null
      update-rc.d -f lfd remove &> /dev/null
      rm -f /etc/cron.d/{csf,lfd}*
      rm -f /usr/sbin/{csf,lfd}
      rm -f /etc/init.d/{csf,lfd}
      rm -rf /etc/csf
    else
      if [ "${_CUSTOM_CONFIG_CSF}" = "YES" ]; then
        _DO_NOTHING=YES
      else
        _CSF_MODE=upgrade
        csf_install
      fi
      if [ -e "/root/.host8.cnf" ] \
        || [ -e "/root/.auto.up.cnf" ] \
        || [[ "${_CHECK_HOST}" =~ ".boa.io" ]] \
        || [[ "${_CHECK_HOST}" =~ ".o8.io" ]] \
        || [[ "${_CHECK_HOST}" =~ ".aegir.cc" ]] \
        || [[ "${_CHECK_HOST}" =~ ".host8." ]]; then
        sed -i "s/^AUTO_UPDATES .*/AUTO_UPDATES = \"1\"/g" \
          /etc/csf/csf.conf &> /dev/null
        wait
      fi
      if [ "${_VMFAMILY}" = "VZ" ]; then
        sed -i "s/^PORTFLOOD .*/PORTFLOOD = \"\"/g" /etc/csf/csf.conf &> /dev/null
        wait
        sed -i "s/^CONNLIMIT .*/CONNLIMIT = \"\"/g" /etc/csf/csf.conf &> /dev/null
        wait
        sed -i "s/^USE_CONNTRACK .*/USE_CONNTRACK = \"0\"/g" \
          /etc/csf/csf.conf &> /dev/null
        wait
      fi
      if [ -e "${pthLog}/lastFire" ]; then
        rm -f ${pthLog}/lastFire
        service lfd stop &> /dev/null
        sleep 3
        kill -9 $(ps aux | grep '[C]onfigServer' | awk '{print $2}') &> /dev/null
        killall sleep &> /dev/null
        rm -f /etc/csf/csf.error
        mrun "csf -x" &> /dev/null
        mrun "csf -e" &> /dev/null
        mrun "csf -q" &> /dev/null
        sleep 8
        service lfd start &> /dev/null
        ### Linux kernel TCP SACK CVEs mitigation
        ### CVE-2019-11477 SACK Panic
        ### CVE-2019-11478 SACK Slowness
        ### CVE-2019-11479 Excess Resource Consumption Due to Low MSS Values
        if [ -e "/usr/sbin/csf" ] && [ -e "/etc/csf/csf.deny" ]; then
          _SACK_TEST=$(ip6tables --list | grep tcpmss 2>&1)
          if [[ ! "${_SACK_TEST}" =~ "tcpmss" ]]; then
            sysctl net.ipv4.tcp_mtu_probing=0 &> /dev/null
            iptables -A INPUT -p tcp -m tcpmss --mss 1:500 -j DROP &> /dev/null
            ip6tables -A INPUT -p tcp -m tcpmss --mss 1:500 -j DROP &> /dev/null
          fi
        fi
      fi
    fi
  fi
}
