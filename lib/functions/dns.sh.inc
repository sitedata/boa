#
# Fix DNS settings.
fix_dns_settings() {
  dnsLi="/root/.local.dns.IP.list"
  mkdir -p ${vBs}
  rm -f ${vBs}/resolv.conf.tmp
  if [ -e "/etc/resolv.conf" ]; then
    if [ -L "/etc/resolv.conf" ]; then
      rslvT=`readlink -n /etc/resolv.conf`
      if [ ! -e "${rslvT}" ]; then
        rm -f /etc/resolv.conf
      fi
    fi
    if [ -e "/etc/resolv.conf" ]; then
      cp -a /etc/resolv.conf ${vBs}/resolv.conf.tmp
    fi
  fi
  if [ ! -e "${vBs}/resolv.conf.tmp" ]; then
    echo "nameserver 1.1.1.1" >${vBs}/resolv.conf.tmp
    echo "nameserver 1.0.0.1" >>${vBs}/resolv.conf.tmp
  fi
  if [ ! -e "${vBs}/resolv.conf.vanilla" ]; then
    for Pre in `ls -la ${vBs}/resolv.conf.pre-*`; do
      if [ -e "${Pre}" ] && [ -f "${Pre}" ] && [ ! -L "${Pre}" ]; then
        cp -a ${Pre} ${vBs}/resolv.conf.vanilla
      fi
    done
  fi
  if [ ! -e "${vBs}/resolv.conf.vanilla" ] \
    && [ -e "${vBs}/resolv.conf.tmp" ]; then
    if [ "${_USE_DEFAULT_DNS}" != "YES" ]; then
      rm -f /etc/resolv.conf
      cp -a ${vBs}/resolv.conf.tmp /etc/resolv.conf
      cp -a ${vBs}/resolv.conf.tmp ${vBs}/resolv.conf.vanilla
    fi
  fi
  sed -i "/^$/d" ${vBs}/resolv.conf.vanilla &> /dev/null
  if [ -e "${vBs}/resolv.conf.vanilla" ]; then
    if [ "${_USE_DEFAULT_DNS}" != "YES" ]; then
      rm -f /etc/resolv.conf
      cp -a ${vBs}/resolv.conf.vanilla /etc/resolv.conf
    fi
  else
    if [ -e "${vBs}/resolv.conf.tmp" ] \
      && [ "${_USE_DEFAULT_DNS}" != "YES" ]; then
      rm -f /etc/resolv.conf
      cp -a ${vBs}/resolv.conf.tmp /etc/resolv.conf
    fi
  fi
  if [ -e "/etc/pdnsd.conf" ]; then
    if [ -e "${dnsLi}" ]; then
      sed -i "s/.*127.0.0.1.*//g; s/ *$//g; /^$/d" ${dnsLi}
      wait
      sed -i "s/.*Dynamic.*//g; s/ *$//g; /^$/d"   ${dnsLi}
      wait
      _BROKEN_DNS_TEST_X=$(grep "\." ${dnsLi} 2>&1)
      if [ -z "${_BROKEN_DNS_TEST_X}" ]; then
        echo "        label = \"cloudflare-servers\";" > ${dnsLi}
        echo "        ip=1.1.1.1;" >> ${dnsLi}
        echo "        ip=1.0.0.1;" >> ${dnsLi}
      fi
    fi
    _CUSTOM_DNS_TEST=$(grep 1.1.1.1 /etc/pdnsd.conf 2>&1)
    _BROKEN_DNS_CONF=$(grep "ip=Dynamic" /etc/pdnsd.conf 2>&1)
    _MISSING_DNS_TEST=$(grep "ip=" /etc/pdnsd.conf 2>&1)
    if [[ "${_CUSTOM_DNS_TEST}" =~ "1.1.1.1" ]] \
      || [ ! -e "${dnsLi}" ] \
      || [ -e "/root/.use.default.nameservers.cnf" ] \
      || [ -e "/root/.use.local.nameservers.cnf" ] \
      || [[ "${_BROKEN_DNS_CONF}" =~ "Dynamic" ]] \
      || [[ -z "${_MISSING_DNS_TEST}" ]]; then
      echo "        label = \"cloudflare-servers\";" > ${dnsLi}
      for _IP in `cat /etc/resolv.conf \
        | sed 's/.*127.0.0.1.*//g; s/.*search.*//g; s/.*Dynamic.*//g' \
        | cut -d ' ' -f2 \
        | sort \
        | uniq`;do echo "        ip=${_IP};" >> ${dnsLi};done
      wait
      sed -i "s/ip=.*//g; s/ *$//g; /^$/d" /etc/pdnsd.conf
      wait
      sed -i "s/.*127.0.0.1.*//g; s/ *$//g; /^$/d" ${dnsLi}
      wait
      sed -i "s/.*Dynamic.*//g; s/ *$//g; /^$/d"   ${dnsLi}
      wait
      _BROKEN_DNS_TEST_Y=$(grep "\." ${dnsLi} 2>&1)
      if [ -z "${_BROKEN_DNS_TEST_Y}" ]; then
         echo "        ip=1.1.1.1;" >> ${dnsLi}
         echo "        ip=1.0.0.1;" >> ${dnsLi}
      fi
      ### echo debug dns A
      _DNS_TPL_TEST_GE=$(grep "google-servers" /etc/pdnsd.conf 2>&1)
      _DNS_TPL_TEST_CF=$(grep "cloudflare-servers" /etc/pdnsd.conf 2>&1)
      _DNS_RGX_TEST=$(grep "cloudflare-servers" /root/.local.dns.IP.list 2>&1)
      if [[ "${_DNS_TPL_TEST_CF}" =~ "cloudflare-servers" ]] \
        && [[ "${_DNS_RGX_TEST}" =~ "cloudflare-servers" ]]; then
        sed -i '/        label = \"cloudflare-servers\";/ {r /root/.local.dns.IP.list
d;};' /etc/pdnsd.conf
        wait
      fi
      if [[ "${_DNS_TPL_TEST_GE}" =~ "google-servers" ]] \
        && [[ "${_DNS_RGX_TEST}" =~ "cloudflare-servers" ]]; then
        sed -i '/        label = \"google-servers\";/ {r /root/.local.dns.IP.list
d;};' /etc/pdnsd.conf
        wait
      fi
      resolvconf -u         &> /dev/null
      service pdnsd restart &> /dev/null
      pdnsd-ctl empty-cache &> /dev/null
    fi
  fi
}

#
# Check DNS settings.
check_dns_settings() {
  if [ -e "/root/.use.default.nameservers.cnf" ]; then
    _USE_DEFAULT_DNS=YES
    rm -f /root/.local.dns.IP.list
  fi
  if [ -e "/root/.use.local.nameservers.cnf" ]; then
    _USE_PROVIDER_DNS=YES
  else
    _REMOTE_DNS_TEST=$(host -s files.aegir.cc 1.1.1.1 -w 10 2>&1)
  fi
  if [[ "${_REMOTE_DNS_TEST}" =~ "no servers could be reached" ]] \
    || [[ "${_REMOTE_DNS_TEST}" =~ "Host files.aegir.cc not found" ]] \
    || [ "${_USE_DEFAULT_DNS}" = "YES" ] \
    || [ "${_USE_PROVIDER_DNS}" = "YES" ]; then
    if [ "${_USE_DEFAULT_DNS}" = "YES" ] \
      || [ "${_USE_PROVIDER_DNS}" = "YES" ] \
      || [ ! -e "${vBs}/resolv.conf.vanilla" ]; then
      fix_dns_settings
      if [ -e "/etc/init.d/postfix" ]; then
        service postfix restart &> /dev/null
      fi
    fi
  fi
}

#
# Check repo status.
check_git_repos() {
  if [ "${_AEGIR_VERSION}" = "HEAD" ]; then
    _GITHUB_WORKS=NO
    _GITLAB_WORKS=NO
    if [ "${_FORCE_GIT_MIRROR}" = "drupal" ]; then
      _FORCE_GIT_MIRROR=github
    fi
    if [ "${_FORCE_GIT_MIRROR}" = "gitorious" ]; then
      _FORCE_GIT_MIRROR=gitlab
    fi
    if [ "${_FORCE_GIT_MIRROR}" = "github" ]; then
      msg "INFO: We will use forced GitHub repository without testing connection"
      _GITHUB_WORKS=YES
      _GITLAB_WORKS=NO
      sleep 1
    elif [ "${_FORCE_GIT_MIRROR}" = "gitlab" ]; then
      msg "INFO: We will use forced GitLab mirror without testing connection"
      _GITHUB_WORKS=NO
      _GITLAB_WORKS=YES
      sleep 1
    else
      if [ "${_DEBUG_MODE}" = "YES" ]; then
        msg "INFO: Testing repository mirror servers availability..."
      fi
      sleep 1
      _GITHUB_WORKS=YES
      _GITLAB_WORKS=YES
      if ! netcat -w 10 -z github.com 443 ; then
        _GITHUB_WORKS=NO
        msg "WARN: The GitHub master repository server doesn't respond..."
      elif ! netcat -w 10 -z gitlab.com 443 ; then
        _GITLAB_WORKS=NO
        msg "WARN: The GitLab mirror repository server doesn't respond..."
      fi
    fi
    if [ "$_GITHUB_WORKS" = "YES" ]; then
      _BOA_REPO_NAME="boa"
      _BOA_REPO_GIT_URL="${gitHub}"
      if [ "${_DEBUG_MODE}" = "YES" ]; then
        msg "INFO: GitHub master repository will be used"
      fi
    elif [ "$_GITLAB_WORKS" = "YES" ]; then
      _BOA_REPO_NAME="boa"
      _BOA_REPO_GIT_URL="${gitLab}"
      if [ "${_DEBUG_MODE}" = "YES" ]; then
        msg "INFO: GitLab mirror repository will be used"
      fi
    else
      cat <<EOF

      None of repository servers responded in 8 seconds,
      so we can't continue this installation.

      Please try again later or check if your firewall has port 443 open.

      Bye.

EOF
      clean_pid_exit
    fi
  fi
}

#
# Find correct IP.
find_correct_ip() {
  _LOC_IP=$(curl ${crlGet} https://api64.ipify.org \
    | sed 's/[^0-9\.]//g' 2>&1)
  if [ -z "${_LOC_IP}" ]; then
    _LOC_IP=$(curl ${crlGet} http://ip4.icanhazip.com \
      | sed 's/[^0-9\.]//g' 2>&1)
  fi
}

#
# Validate server public IP.
validate_public_ip() {
  if [ "${_VMFAMILY}" = "AWS" ]; then
    _LOC_DOM="${_MY_HOSTN}"
    if [ -z "${_MY_OWNIP}" ]; then
      find_correct_ip
      _MY_OWNIP="${_LOC_IP}"
    else
      _LOC_IP="${_MY_OWNIP}"
    fi
  fi
  if [ ! -z "${_LOCAL_NETWORK_IP}" ]; then
    if [ -z "${_LOCAL_NETWORK_HN}" ]; then
      msg "FATAL ERROR: you must specify also _LOCAL_NETWORK_HN"
      clean_pid_exit
    else
      _MY_OWNIP="${_LOCAL_NETWORK_IP}"
      _MY_HOSTN="${_LOCAL_NETWORK_HN}"
      _MY_FRONT="${_LOCAL_NETWORK_HN}"
      _THISHTIP="${_LOCAL_NETWORK_IP}"
    fi
  else
    if [ "${_DNS_SETUP_TEST}" = "YES" ]; then
      if [ -z "${_MY_OWNIP}" ]; then
        find_correct_ip
        _THISHTIP="${_LOC_IP}"
      else
        _THISHTIP="${_MY_OWNIP}"
      fi
    else
      if [ -z "${_MY_OWNIP}" ] && [ ! -z "${_MY_HOSTN}" ]; then
        _LOC_DOM="${_MY_HOSTN}"
        find_correct_ip
        _THISHTIP="${_LOC_IP}"
      else
        _THISHTIP="${_MY_OWNIP}"
      fi
    fi
  fi
}

#
# Validate server IP for xtras.
validate_xtras_ip() {
  if [ "${_VMFAMILY}" = "AWS" ]; then
    _LOC_DOM="${_MY_HOSTN}"
    if [ -z "${_MY_OWNIP}" ]; then
      find_correct_ip
      _MY_OWNIP="${_LOC_IP}"
    else
      _LOC_IP="${_MY_OWNIP}"
    fi
  fi
  _XTRAS_THISHTIP="*"
}

#
# Validate server IP for purge vhost.
validate_purge_ip() {
  if [ "${_PURGE_ALL_THISHTIP}" = "YES" ]; then
    _PURGE_THISHTIP="0.0.0.0/0"
  else
    if [ "${_DNS_SETUP_TEST}" = "YES" ]; then
      if [ -z "${_MY_OWNIP}" ]; then
        find_correct_ip
        _PURGE_THISHTIP="${_LOC_IP}"
      else
        _PURGE_THISHTIP="${_MY_OWNIP}"
      fi
    else
      if [ -z "${_MY_OWNIP}" ]; then
        if [ -e "/usr/bin/sipcalc" ]; then
          if [ -z "${_THISHTIP}" ]; then
            _LOC_DOM="${_THISHOST}"
            find_correct_ip
            _THISHTIP="${_LOC_IP}"
          fi
          _IP_TEST=$(sipcalc ${_THISHTIP} 2>&1)
          if [[ "${_IP_TEST}" =~ "ERR" ]]; then
            _IP_TEST_RESULT=FAIL
            _PURGE_THISHTIP="0.0.0.0/0"
          else
            _IP_TEST_RESULT=OK
            _PURGE_THISHTIP="${_THISHTIP}"
          fi
        else
          _PURGE_THISHTIP="${_THISHTIP}"
        fi
      else
        _PURGE_THISHTIP="${_MY_OWNIP}"
      fi
    fi
    if [ -z "${_PURGE_THISHTIP}" ]; then
      _PURGE_THISHTIP="0.0.0.0/0"
    fi
  fi
}

#
# Validate local server IP.
validate_local_ip() {
  _LOCAL_THISHTIP=all
}

#
# Wait for connection.
wait_for_connection() {
  echo " "
  msg "I can not connect to github.com on port 443 at the moment"
  msg "I will try again in 60 seconds, please wait..."
  msg "Waiting for attempt $1..."
  sleep 60
}

#
# Check connection.
check_connection() {
  if ! netcat -w 10 -z github.com 443 ; then
    wait_for_connection "2/4"
    if ! netcat -w 10 -z github.com 443 ; then
      wait_for_connection "3/4"
      if ! netcat -w 10 -z github.com 443 ; then
        wait_for_connection "4/4"
        if ! netcat -w 10 -z github.com 443 ; then
          echo " "
          msg "Sorry, I gave up."
          msg "EXIT on error due to GitHub git server at 443 downtime"
          msg "Please try to run this script again in a few minutes"
          msg "You may want to check https://status.github.com/messages"
          msg "Also, make sure that the outgoing connections via port 443 work"
          msg "Bye"
          clean_pid_exit
        fi
      fi
    fi
  fi
}

install_pdnsd_cache() {
  msg "INFO: Installing DNS cache pdnsd server..."
  cp -af /etc/resolv.conf ${vBs}/resolv.conf.pre-${_X_VERSION}-${_NOW}
  if [ "${_USE_PROVIDER_DNS}" != "YES" ]; then
    rm -f /etc/resolv.conf
    echo "nameserver 1.1.1.1" > /etc/resolv.conf
    echo "nameserver 1.0.0.1" >> /etc/resolv.conf
  fi
  st_runner "apt-get update -qq" &> /dev/null
  st_runner "aptitude purge pdnsd -y" &> /dev/null
  st_runner "aptitude purge resolvconf -y" &> /dev/null
  rm -rf /lib/init/rw/resolvconf
  rm -rf /etc/resolvconf
  st_runner "aptitude purge pdnsd -y" &> /dev/null
  st_runner "aptitude purge resolvconf -y" &> /dev/null
  if [ "${_USE_PROVIDER_DNS}" != "YES" ]; then
    rm -f /etc/resolv.conf
    echo "nameserver 1.1.1.1" > /etc/resolv.conf
    echo "nameserver 1.0.0.1" >> /etc/resolv.conf
  fi
  if [ "${_OSR}" = "buster" ] \
    || [ "${_OSR}" = "stretch" ]; then
    st_runner "${_INSTALL} resolvconf" &> /dev/null
    msg "INFO: Building pdnsd ${_PDNSD_VRN} from sources, please wait..."
    cd /var/opt
    rm -rf pdnsd*
    get_dev_src "pdnsd-${_PDNSD_VRN}.tar.gz"
    cd /var/opt/pdnsd-${_PDNSD_VRN}
    st_runner "bash ./configure --prefix=/usr" 2> /dev/null
    mrun "make --quiet" 2> /dev/null
    mrun "make --quiet install" 2> /dev/null
    mrun "ldconfig" 2> /dev/null
    echo "START_DAEMON=yes" > /etc/default/pdnsd
    adduser --system --ingroup proxy --home /var/cache/pdnsd pdnsd &> /dev/null
    usermod -aG proxy pdnsd
    cp -af ${locCnf}/pdnsd /etc/init.d/pdnsd
    cat ${locCnf}/pdnsd.conf > /etc/pdnsd.conf
    ln -sf /etc/pdnsd.conf /usr/etc/pdnsd.conf
    chmod 755 /etc/init.d/pdnsd
    mrun "update-rc.d pdnsd defaults" &> /dev/null
    mrun "service pdnsd start" &> /dev/null
  else
    st_runner "${_INSTALL} resolvconf" &> /dev/null
    st_runner "${_INSTALL} pdnsd" &> /dev/null
    st_runner "${_INSTALL} resolvconf" &> /dev/null
    st_runner "${_INSTALL} pdnsd" &> /dev/null
    sed -i "s/START_DAEMON=no/START_DAEMON=yes/g" /etc/default/pdnsd &> /dev/null
    cat ${locCnf}/pdnsd.conf > /etc/pdnsd.conf
  fi
  if [ -x "/usr/sbin/pdnsd" ] \
    && [ ! -e "/etc/resolvconf/run/interface/lo.pdnsd" ]; then
    mkdir -p /etc/resolvconf/run/interface
    echo "nameserver 127.0.0.1" > /etc/resolvconf/run/interface/lo.pdnsd
    resolvconf -u         &> /dev/null
    service pdnsd restart &> /dev/null
    pdnsd-ctl empty-cache &> /dev/null
  fi
  check_dns_settings
  if [ ! -L "/etc/resolvconf/run" ] \
    || [ ! -e "/lib/init/rw/resolvconf/enable-updates" ] \
    || [ ! -e "/etc/resolvconf/resolv.conf.d/tail" ]; then
    rm -rf /run/resolvconf
    rm -rf /etc/resolvconf/run
    ln -s /lib/init/rw/resolvconf /etc/resolvconf/run
    if [ ! -e "/lib/init/rw/resolvconf/interface" ]; then
      mkdir -p /lib/init/rw/resolvconf/interface
    fi
    mkdir -p /run/resolvconf
    ln -s /lib/init/rw/resolvconf/interface /run/resolvconf/interface
    rm -f /lib/init/rw/resolvconf/postponed-update
    touch /lib/init/rw/resolvconf/enable-updates
    rm -f /etc/resolvconf/resolv.conf.d/tail
    touch /etc/resolvconf/resolv.conf.d/tail
  fi
  if [ -x "/usr/sbin/pdnsd" ] \
    && [ ! -e "/etc/resolvconf/run/interface/lo.pdnsd" ]; then
    mkdir -p /etc/resolvconf/run/interface
    echo "nameserver 127.0.0.1" > /etc/resolvconf/run/interface/lo.pdnsd
    resolvconf -u         &> /dev/null
    service pdnsd restart &> /dev/null
    pdnsd-ctl empty-cache &> /dev/null
  fi
  if [ "${_USE_PROVIDER_DNS}" = "YES" ] \
    && [ -e "${vBs}/resolv.conf.vanilla" ]; then
    cat ${vBs}/resolv.conf.vanilla > /etc/resolvconf/resolv.conf.d/base
  fi
  resolvconf -u &> /dev/null
  if [ -e "/etc/resolvconf/run/resolv.conf" ]; then
    _RESOLV_TEST=$(grep "nameserver 127.0.0.1" /etc/resolv.conf 2>&1)
    if [[ "$_RESOLV_TEST" =~ "nameserver 127.0.0.1" ]]; then
      _DO_NOTHING=YES
    else
      rm -f /etc/resolv.conf
      ln -s /etc/resolvconf/run/resolv.conf /etc/resolv.conf
      resolvconf -u         &> /dev/null
      service pdnsd restart &> /dev/null
      pdnsd-ctl empty-cache &> /dev/null
    fi
  fi
  if [ ! -e "/etc/resolv.conf" ]; then
    rm -f /etc/resolv.conf
    if [ -e "${vBs}/resolv.conf.vanilla" ]; then
      cat ${vBs}/resolv.conf.vanilla >/etc/resolv.conf
    fi
    echo "nameserver 127.0.0.1" >> /etc/resolv.conf
    echo "nameserver 1.1.1.1" >>/etc/resolv.conf
    echo "nameserver 1.0.0.1" >>/etc/resolv.conf
  fi
  if [ -e "/etc/NetworkManager/NetworkManager.conf" ]; then
    sed -i "s/^dns=.*/dns=pdnsd/g" \
      /etc/NetworkManager/NetworkManager.conf &> /dev/null
    mrun "service network-manager restart" &> /dev/null
  fi
  resolvconf -u         &> /dev/null
  service pdnsd restart &> /dev/null
  pdnsd-ctl empty-cache &> /dev/null
}

check_github_for_aegir_head_mode() {
  if [ "${_SYSTEM_UPGRADE_ONLY}" = "NO" ] \
    && [ "${_AEGIR_VERSION}" = "HEAD" ]; then
    rm -rf /opt/tmp/test-*
    check_connection
    _GITHUB_TEST=$(git clone ${gitHub}/provision.git \
      /opt/tmp/test-provision 2>&1)
    if [[ "${_GITHUB_TEST}" =~ "fatal" ]]; then
      echo " "
      msg "EXIT on error (provision) due to GitHub downtime"
      msg "Please try to run this script again in a few minutes"
      msg "You may want to check https://status.github.com/messages"
      msg "Bye"
      rm -rf /opt/tmp/test-*
      clean_pid_exit
    fi
    _GITHUB_TEST=$(git clone ${gitHub}/hostmaster.git \
      /opt/tmp/test-hostmaster 2>&1)
    if [[ "${_GITHUB_TEST}" =~ "fatal" ]]; then
      echo " "
      msg "EXIT on error (hostmaster) due to GitHub downtime"
      msg "Please try to run this script again in a few minutes"
      msg "You may want to check https://status.github.com/messages"
      msg "Bye"
      rm -rf /opt/tmp/test-*
      clean_pid_exit
    fi
    rm -rf /opt/tmp/test-*
  fi
  if [[ "${_X_VERSION}" =~ "-dev" ]] \
    || [[ "${_X_VERSION}" =~ "-rel" ]]; then
    rm -rf /opt/tmp/test-*
    _GITHUB_TEST=$(git clone ${gitHub}/boa.git \
      /opt/tmp/test-boa 2>&1)
    if [[ "${_GITHUB_TEST}" =~ "fatal" ]]; then
      echo " "
      msg "EXIT on error (boa) due to GitHub downtime"
      msg "Please try to run this script again in a few minutes"
      msg "You may want to check https://status.github.com/messages"
      msg "Bye"
      rm -rf /opt/tmp/test-*
      clean_pid_exit
    fi
    rm -rf /opt/tmp/test-*
  fi
}

check_db_src() {
  if ! netcat -w 14 -z ${_DB_SRC} 80 ; then
    echo " "
    msg "EXIT on error due to ${_DB_SRC} downtime"
    msg "Please try to run this script again in a few minutes"
    msg "or better yet, hours"
    msg "Bye"
    clean_pid_exit
  fi
}

check_ip_hostname() {
  if [ "${_STATUS}" = "INIT" ]; then
    if [ ! -z "${_LOCAL_NETWORK_IP}" ]; then
      if [ -z "${_LOCAL_NETWORK_HN}" ]; then
        msg "FATAL ERROR: you must specify also _LOCAL_NETWORK_HN"
        clean_pid_exit
      else
        _DNS_SETUP_TEST=NO
        _SMTP_RELAY_TEST=NO
        _MY_OWNIP="${_LOCAL_NETWORK_IP}"
        _MY_HOSTN="${_LOCAL_NETWORK_HN}"
        _MY_FRONT="${_LOCAL_NETWORK_HN}"
      fi
    fi
    if [ "${_VMFAMILY}" = "AWS" ]; then
      _THIS_DB_HOST=localhost
      _LOC_DOM="${_MY_HOSTN}"
      if [ -z "${_MY_OWNIP}" ]; then
        find_correct_ip
        _MY_OWNIP="${_LOC_IP}"
      else
        _LOC_IP="${_MY_OWNIP}"
      fi
    fi
    if [ ! -z "${_MY_OWNIP}" ]; then
      if [ ! -z "${_MY_HOSTN}" ]; then
        _S_N=${_MY_HOSTN}
        _S_T=${_S_N#*.*}
        _S_Q=${_S_N%%${_S_T}}
        _S_E=${_S_Q%*.*}
        if [ ! -z "${_LOCAL_NETWORK_HN}" ]; then
          if [ "${_EASY_SETUP}" = "LOCAL" ]; then
            _DO_NOTHING=YES
          else
            sed -i "s/^${_MY_OWNIP}.*//g" /etc/hosts &> /dev/null
            echo "${_MY_OWNIP} ${_MY_HOSTN} chive.${_MY_HOSTN} sqlbuddy.${_MY_HOSTN} cgp.${_MY_HOSTN} $_S_E" >> /etc/hosts
          fi
        fi
        hostname -b ${_MY_HOSTN} ### force our custom FQDN/local hostname
        echo "${_MY_HOSTN}" > /etc/hostname
        echo "${_MY_HOSTN}" > /etc/mailname
      fi
      _THISHTIP="${_MY_OWNIP}"
      _THISHOST="${_MY_HOSTN}"
      _LOC_DOM="${_THISHOST}"
      find_correct_ip
      _THISRDIP="${_LOC_IP}"
      if [ "${_THISRDIP}" = "${_THISHTIP}" ]; then
        _FQDNTEST="TRUE"
        _LOC_DOM="${_MY_FRONT}"
        find_correct_ip
        _THEFRDIP="${_LOC_IP}"
        if [ "$_THEFRDIP" = "${_THISHTIP}" ]; then
          _TESTHOST=$(uname -n 2>&1)
          _LOC_DOM="$_TESTHOST"
          find_correct_ip
          _TESTRDIP="${_LOC_IP}"
          if [ "$_TESTRDIP" = "${_THISHTIP}" ]; then
            _FQDNTEST="TRUE"
            hostname -b $_TESTHOST
          else
           _FQDNTEST="FALSE"
          fi
        else
          _FQDNTEST="FALSE"
        fi
      else
        _FQDNTEST="FALSE"
      fi
    else
      find_correct_ip
      _THISHTIP="${_LOC_IP}"
      _FQDNPROB=$(uname -n 2>&1)
      _FQDNTEST="FALSE"
      _THISHOST=$(uname -n 2>&1)
      if [ ! -z "$_FQDNPROB" ]; then
        _THISHOST=$(uname -n 2>&1)
        _THISHOST=${_THISHOST//[^a-zA-Z0-9-.]/}
        _THISHOST=$(echo -n ${_THISHOST} | tr A-Z a-z 2>&1)
        _LOC_DOM="${_THISHOST}"
        find_correct_ip
        _THISRDIP="${_LOC_IP}"
        if [ "${_THISRDIP}" = "${_THISHTIP}" ]; then
          _FQDNTEST="TRUE"
          hostname -b ${_THISHOST}
        else
          _FQDNTEST="FALSE"
          _REVHOSTN=$(host ${_THISHTIP} | cut -d: -f2 | awk '{ print $5}' 2>&1)
          _REVHOSTN=$(echo -n $_REVHOSTN |sed 's/\(.*\)./\1/' 2>&1)
          _REVHOSTN=${_REVHOSTN//[^a-zA-Z0-9-.]/}
          _REVHOSTN=$(echo -n $_REVHOSTN | tr A-Z a-z 2>&1)
          _LOC_DOM="$_REVHOSTN"
          find_correct_ip
          _REVHSTIP="${_LOC_IP}"
          if [ "$_REVHSTIP" = "${_THISHTIP}" ]; then
            hostname -b $_REVHOSTN
            _THISHOST="$_REVHOSTN"
            _FQDNTEST="TRUE"
          else
            _FQDNTEST="FALSE"
          fi
        fi
      else
        _REVHOSTN=$(host ${_THISHTIP} | cut -d: -f2 | awk '{ print $5}' 2>&1)
        _REVHOSTN=$(echo -n $_REVHOSTN |sed 's/\(.*\)./\1/' 2>&1)
        _REVHOSTN=${_REVHOSTN//[^a-zA-Z0-9-.]/}
        _REVHOSTN=$(echo -n $_REVHOSTN | tr A-Z a-z 2>&1)
        _LOC_DOM="$_REVHOSTN"
        find_correct_ip
        _REVHSTIP="${_LOC_IP}"
        if [ "$_REVHSTIP" = "${_THISHTIP}" ]; then
          hostname -b $_REVHOSTN
          _THISHOST="$_REVHOSTN"
          _FQDNTEST="TRUE"
        else
         _FQDNTEST="FALSE"
        fi
      fi
    fi
    if [ ! -z "${_MY_FRONT}" ]; then
      _THIS_FRONT="${_MY_FRONT}"
    else
      _THIS_FRONT="${_THISHOST}"
    fi
    if [ "${_DNS_SETUP_TEST}" = "NO" ]; then
      _FQDNTEST=TRUE
    fi
    if [ "${_THISHOST}" = "localhost" ]; then
      msg "FATAL ERROR: you can't use localhost as your FQDN hostname"
      msg "Please try something like: aegir.local"
      clean_pid_exit
    fi
    if [ "$_FQDNTEST" = "FALSE" ]; then
      echo " "
      msg "EXIT on error due to invalid DNS setup"
      if [ ! -z "${_MY_OWNIP}" ]; then
        cat <<EOF

    * Your custom _MY_OWNIP is set to "${_MY_OWNIP}"
    * Your custom _MY_HOSTN is set to "${_MY_HOSTN}"
    * Your custom _MY_FRONT is set to "${_MY_FRONT}"

    * Your _MY_HOSTN and/or _MY_FRONT doesn't match your _MY_OWNIP,
      or your hostname is not set properly yet.

    * Please make sure that below command returns your FQDN hostname "${_MY_HOSTN}":

    $ uname -n

EOF
      fi
      cat <<EOF

    Your server needs a working FQDN hostname pointing to its IP address.
    This means that you have to configure DNS for your hostname before
    trying to install BOA. Reverse DNS is not required, though.
    Make sure that DNS A record for ${_THISHOST} points to ${_THISHTIP} and
    then allow some time for DNS propagation before trying this again.
    Alternatively, disable this check with _DNS_SETUP_TEST=NO

EOF
      msg "EXIT on error due to invalid DNS setup"
      clean_pid_exit
    else
      echo "${_THISHOST}" > /etc/hostname
      echo "${_THISHOST}" > /etc/mailname
      hostname -b ${_THISHOST}
      msg "INFO: DNS test: OK"
    fi
    echo " "
    msg "INSTALL START -> checkpoint: "
    cat <<EOF

    * Your email address is ${_MY_EMAIL}
    * Your IP address is ${_THISHTIP}
    * Your hostname is ${_THISHOST}
EOF
    echo " "
    if prompt_yes_no "Do you want to proceed with the install?" ; then
      true
    else
      echo "Installation aborted by you"
      clean_pid_exit
    fi
  else ### if [ "${_STATUS}" = "UPGRADE" ]; then
    _THISHOST=$(cat /etc/hostname 2>&1)
    _THISHOST=$(echo -n ${_THISHOST} | tr -d "\n" 2>&1)
    _THISHOST=${_THISHOST//[^a-zA-Z0-9-.]/}
    _THISHOST=$(echo -n ${_THISHOST} | tr A-Z a-z 2>&1)
    if [ -e "/var/aegir/.drush/hostmaster.alias.drushrc.php" ]; then
      _THIS_FRONT=$(cat /var/aegir/.drush/hostmaster.alias.drushrc.php \
        | grep "uri'" \
        | cut -d: -f2 \
        | awk '{ print $3}' \
        | sed "s/[\,']//g" 2>&1)
    elif [ ! -z "${_MY_FRONT}" ]; then
      _THIS_FRONT="${_MY_FRONT}"
    else
      msg "EXIT on error due to not found or not specified Aegir domain"
      msg "Please specify your working Aegir domain as a value of _MY_FRONT"
      clean_pid_exit
    fi
    if [ "${_DEBUG_MODE}" = "YES" ]; then
      echo " "
      msg "UPGRADE START -> checkpoint: "
      cat <<EOF

    * Your email address is ${_MY_EMAIL}
    * Your server hostname is ${_THISHOST}
    * Your Aegir control panel URL is https://${_THIS_FRONT}
EOF
      echo " "
    fi
    if [ "${_DEBUG_MODE}" = "YES" ]; then
      if prompt_yes_no "Do you want to proceed with the upgrade?" ; then
        true
      else
        echo "Upgrade aborted by you"
        clean_pid_exit
      fi
    fi
  fi
}

install_upgrade_pdnsd() {
  if [ -f "/etc/default/dhcpcd" ]; then
    sed -i "s/.*SET_DNS/#SET_DNS/g" /etc/default/dhcpcd &> /dev/null
    wait
    sed -i "s/.*SET_HOSTNAME/#SET_HOSTNAME/g" /etc/default/dhcpcd &> /dev/null
    wait
  fi
  check_dns_settings
  if [ ! -x "/usr/sbin/pdnsd" ]; then
    install_pdnsd_cache
  else
    if [ "${_USE_PROVIDER_DNS}" = "YES" ] \
      || [ "${_USE_DEFAULT_DNS}" = "YES" ] \
      || [ ! -L "/etc/resolvconf/run" ] \
      || [ ! -e "/lib/init/rw/resolvconf/enable-updates" ] \
      || [ ! -e "/etc/resolvconf/run/interface/lo.pdnsd" ]; then
      msg "INFO: DNS cache pdnsd server re-installation forced"
      install_pdnsd_cache
    fi
  fi
}

if_install_upgrade_pdnsd() {
  if [ -r "/etc/pdnsd.conf" ] || [[ "${_XTRAS_LIST}" =~ "DCS" ]]; then
    install_upgrade_pdnsd
  fi
}
