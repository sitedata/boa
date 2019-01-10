#!/bin/bash

PATH=/usr/local/bin:/usr/local/sbin:/opt/local/bin:/usr/bin:/usr/sbin:/bin:/sbin
SHELL=/bin/bash

check_root() {
  if [ `whoami` = "root" ]; then
    ionice -c2 -n7 -p $$
    chmod a+w /dev/null
    if [ ! -e "/dev/fd" ]; then
      if [ -e "/proc/self/fd" ]; then
        rm -rf /dev/fd
        ln -s /proc/self/fd /dev/fd
      fi
    fi
  else
    echo "ERROR: This script should be ran as a root user"
    exit 1
  fi
  _DF_TEST=$(df -kTh / -l \
    | grep '/' \
    | sed 's/\%//g' \
    | awk '{print $6}' 2> /dev/null)
  _DF_TEST=${_DF_TEST//[^0-9]/}
  if [ ! -z "${_DF_TEST}" ] && [ "${_DF_TEST}" -gt "90" ]; then
    echo "ERROR: Your disk space is almost full !!! ${_DF_TEST}/100"
    echo "ERROR: We can not proceed until it is below 90/100"
    exit 1
  fi
}
check_root

if [ -e "/root/.proxy.cnf" ]; then
  exit 0
fi

_WEBG=www-data
_X_SE="3.2.2-stable"
_OSV=$(lsb_release -sc 2>&1)
_SSL_ITD=$(openssl version 2>&1 \
  | tr -d "\n" \
  | cut -d" " -f2 \
  | awk '{ print $1}')
if [[ "${_SSL_ITD}" =~ "1.0.1" ]] \
  || [[ "${_SSL_ITD}" =~ "1.0.2" ]]; then
  _NEW_SSL=YES
fi
crlGet="-L --max-redirs 10 -k -s --retry 10 --retry-delay 5 -A iCab"
forCer="-fuy --force-yes --reinstall"
vSet="vset --always-set"

###-------------SYSTEM-----------------###

check_config_diff() {
  # $1 is template path
  # $2 is a path to core config
  preCnf="$1"
  myCnf="$2"
  if [ -f "${preCnf}" ] && [ -f "${myCnf}" ]; then
    myCnfUpdate=NO
    diffMyTest=$(diff -w -B ${myCnf} ${preCnf} 2>&1)
    if [ -z "${diffMyTest}" ]; then
      myCnfUpdate=""
      echo "INFO: ${myCnf} diff0 empty -- nothing to update"
    else
      myCnfUpdate=YES
      # diffMyTest=$(echo -n ${diffMyTest} | fmt -su -w 2500 2>&1)
      echo "INFO: ${myCnf} diff1 ${diffMyTest}"
    fi
  fi
}

write_solr_config() {
  # ${1} is module
  # ${2} is a path to solr.php
  # ${3} is Jetty/Solr version
  if [ ! -z "${1}" ] \
    && [ ! -z ${2} ] \
    && [ ! -z "${_MD5H}" ] \
    && [ -e "${Dir}" ]; then
    if [ "${3}" = "jetty97" ]; then
      _PRT="9077"
      _VRS="7.6.0"
    else
      _PRT="8099"
      _VRS="4.9.1"
    fi
    echo "Your SOLR core access details for ${Dom} site are as follows:"  > ${2}
    echo                                                                 >> ${2}
    echo "  Solr version .....: ${_VRS}"                                 >> ${2}
    echo "  Solr host ........: 127.0.0.1"                               >> ${2}
    echo "  Solr port ........: ${_PRT}"                                 >> ${2}
    echo "  Solr path ........: /solr/${_MD5H}.${Dom}.${_HM_U}"          >> ${2}
    echo                                                                 >> ${2}
    echo "It has been auto-configured to work with latest version"       >> ${2}
    echo "of ${1} module, but you need to add the module to"             >> ${2}
    echo "your site codebase before you will be able to use Solr."       >> ${2}
    echo                                                                 >> ${2}
    echo "To learn more please make sure to check the module docs at:"   >> ${2}
    echo                                                                 >> ${2}
    echo "https://drupal.org/project/${1}"                               >> ${2}
    chown ${_HM_U}:users ${2} &> /dev/null
    chmod 440 ${2} &> /dev/null
  fi
}

update_solr() {
  # ${1} is module
  # ${2} is solr core path (auto) == _SOLR_DIR
  if [ ! -z "${1}" ] \
    && [ -e "/var/xdrago/conf/solr" ]; then
    _SOLR_DIR=""
    _JTTY_FAM=""
    if [ "${1}" = "apachesolr" ]; then
      _JTTY_FAM="etty9"
      if [ -e "${Plr}/modules/o_contrib_seven" ]; then
        _SOLR_DIR="/opt/solr4/${_HM_U}.${Dom}"
        if [ ! -e "${_SOLR_DIR}/conf/.protected.conf" ] && [ -e "${_SOLR_DIR}/conf" ]; then
          myCnfUpdate=""
          check_config_diff "/var/xdrago/conf/solr/apachesolr/7/schema.xml" "${_SOLR_DIR}/conf/schema.xml"
          if [ ! -z "${myCnfUpdate}" ]; then
            cp -af /var/xdrago/conf/solr/apachesolr/7/schema.xml ${_SOLR_DIR}/conf/
            cp -af /var/xdrago/conf/solr/apachesolr/7/solrconfig.xml ${_SOLR_DIR}/conf/
            cp -af /var/xdrago/conf/solr/apachesolr/7/solrcore.properties ${_SOLR_DIR}/conf/
            chmod 644 ${_SOLR_DIR}/conf/*
            touch ${_SOLR_DIR}/conf/yes-update.txt
          else
            rm -f ${_SOLR_DIR}/conf/yes-update.txt
          fi
        fi
      else
        _SOLR_DIR="/opt/solr4/${_HM_U}.${Dom}"
        if [ ! -e "${_SOLR_DIR}/conf/.protected.conf" ] && [ -e "${_SOLR_DIR}/conf" ]; then
          myCnfUpdate=""
          check_config_diff "/var/xdrago/conf/solr/apachesolr/6/schema.xml" "${_SOLR_DIR}/conf/schema.xml"
          if [ ! -z "${myCnfUpdate}" ]; then
            cp -af /var/xdrago/conf/solr/apachesolr/6/schema.xml ${_SOLR_DIR}/conf/
            cp -af /var/xdrago/conf/solr/apachesolr/6/solrconfig.xml ${_SOLR_DIR}/conf/
            cp -af /var/xdrago/conf/solr/apachesolr/6/solrcore.properties ${_SOLR_DIR}/conf/
            chmod 644 ${_SOLR_DIR}/conf/*
            touch ${_SOLR_DIR}/conf/yes-update.txt
          else
            rm -f ${_SOLR_DIR}/conf/yes-update.txt
          fi
        fi
      fi
    elif [ "${1}" = "search_api_solr" ] \
      && [ -e "${Plr}/modules/o_contrib_seven" ]; then
      _JTTY_FAM="etty97"
      _SOLR_DIR="/opt/solr7/${_HM_U}.${Dom}"
      if [ ! -e "${_SOLR_DIR}/conf/.protected.conf" ] && [ -e "${_SOLR_DIR}/conf" ]; then
        myCnfUpdate=""
        check_config_diff "/var/xdrago/conf/solr/apachesolr/7/schema.xml" "${_SOLR_DIR}/conf/schema.xml"
        if [ ! -z "${myCnfUpdate}" ]; then
          cp -af /var/xdrago/conf/solr/search_api_solr/7/schema.xml ${_SOLR_DIR}/conf/
          cp -af /var/xdrago/conf/solr/search_api_solr/7/solrconfig.xml ${_SOLR_DIR}/conf/
          cp -af /var/xdrago/conf/solr/search_api_solr/7/solrcore.properties ${_SOLR_DIR}/conf/
          chmod 644 ${_SOLR_DIR}/conf/*
          touch ${_SOLR_DIR}/conf/yes-update.txt
        else
          rm -f ${_SOLR_DIR}/conf/yes-update.txt
        fi
      fi
    elif [ "${1}" = "search_api_solr" ] \
      && [ -e "${Plr}/sites/${Dom}/files/solr/schema.xml" ] \
      && [ -e "${Plr}/sites/${Dom}/files/solr/solrconfig.xml" ] \
      && [ -e "${Plr}/modules/o_contrib_eight" ]; then
      _JTTY_FAM="etty97"
      _SOLR_DIR="/opt/solr7/${_HM_U}.${Dom}"
      if [ ! -e "${_SOLR_DIR}/conf/.protected.conf" ] && [ -e "${_SOLR_DIR}/conf" ]; then
        myCnfUpdate=""
        check_config_diff "${Plr}/sites/${Dom}/files/solr/schema.xml" "${_SOLR_DIR}/conf/schema.xml"
        if [ ! -z "${myCnfUpdate}" ]; then
          mv -f ${Plr}/sites/${Dom}/files/solr/schema.xml ${_SOLR_DIR}/conf/
          mv -f ${Plr}/sites/${Dom}/files/solr/solrconfig.xml ${_SOLR_DIR}/conf/
          rm -f ${Plr}/sites/${Dom}/files/solr/*
          cp -af /var/xdrago/conf/solr/search_api_solr/8/solrcore.properties ${_SOLR_DIR}/conf/
          chmod 644 ${_SOLR_DIR}/conf/*
          touch ${_SOLR_DIR}/conf/yes-update.txt
        else
          rm -f ${_SOLR_DIR}/conf/yes-update.txt
        fi
      fi
    elif [ "${1}" = "search_api_solr" ] \
      && [ ! -e "${Plr}/sites/${Dom}/files/solr/schema.xml" ] \
      && [ -e "${Plr}/modules/o_contrib_eight" ]; then
      _JTTY_FAM="etty97"
      _SOLR_DIR="/opt/solr7/${_HM_U}.${Dom}"
      if [ ! -e "${_SOLR_DIR}/conf/.protected.conf" ] && [ -e "${_SOLR_DIR}/conf" ]; then
        myCnfUpdate=""
        check_config_diff "/var/xdrago/conf/solr/apachesolr/8/schema.xml" "${_SOLR_DIR}/conf/schema.xml"
        if [ ! -z "${myCnfUpdate}" ]; then
          cp -af /var/xdrago/conf/solr/search_api_solr/8/schema.xml ${_SOLR_DIR}/conf/
          cp -af /var/xdrago/conf/solr/search_api_solr/8/solrconfig.xml ${_SOLR_DIR}/conf/
          cp -af /var/xdrago/conf/solr/search_api_solr/8/solrcore.properties ${_SOLR_DIR}/conf/
          chmod 644 ${_SOLR_DIR}/conf/*
          touch ${_SOLR_DIR}/conf/yes-update.txt
        else
          rm -f ${_SOLR_DIR}/conf/yes-update.txt
        fi
      fi
    fi
    if [ -e "${_SOLR_DIR}/conf/yes-update.txt" ]; then
      if [ -e "/etc/default/j${_JTTY_FAM}" ] && [ -e "/etc/init.d/j${_JTTY_FAM}" ]; then
        fiLe="${Dir}/solr.php"
        write_solr_config "${1}" "${fiLe}" "j${_JTTY_FAM}"
        echo "Updated Solr with ${1} for ${_SOLR_DIR}"
        touch ${_SOLR_DIR}/conf/${_X_SE}.conf
        kill -9 $(ps aux | grep '[j]${_JTTY_FAM}' | awk '{print $2}') &> /dev/null
        service j${_JTTY_FAM} start &> /dev/null
      fi
    fi
  fi
}

add_solr() {
  # ${1} is module
  # ${2} is solr core path
  if [ "${1}" = "apachesolr" ]; then
    _SOLR_BASE="/opt/solr4"
  elif [ "${1}" = "search_api_solr" ] \
    && [ -e "${Plr}/modules/o_contrib_seven" ]; then
    _SOLR_BASE="/opt/solr7"
  elif [ "${1}" = "search_api_solr" ] \
    && [ -e "${Plr}/modules/o_contrib_eight" ]; then
    _SOLR_BASE="/opt/solr7"
  fi
  if [ ! -z "${1}" ] && [ ! -z "${2}" ] && [ -e "/var/xdrago/conf/solr" ]; then
    if [ ! -e "${2}" ]; then
      rm -rf ${_SOLR_BASE}/core0/data/*
      cp -a ${_SOLR_BASE}/core0 ${2}
      CHAR="[:alnum:]"
      rkey="32"
      if [ "${_NEW_SSL}" = "YES" ] \
        || [ "${_OSV}" = "jessie" ] \
        || [ "${_OSV}" = "stretch" ] \
        || [ "${_OSV}" = "trusty" ] \
        || [ "${_OSV}" = "precise" ]; then
        _MD5H=$(cat /dev/urandom \
          | tr -cd "$CHAR" \
          | head -c ${1:-$rkey} \
          | openssl md5 \
          | awk '{ print $2}' \
          | tr -d "\n" 2>&1)
      else
        _MD5H=$(cat /dev/urandom \
          | tr -cd "$CHAR" \
          | head -c ${1:-$rkey} \
          | openssl md5 \
          | tr -d "\n" 2>&1)
      fi
      sed -i "s/.*<core name=\"core0\" instanceDir=\"core0\" \/>.*/<core name=\"core0\" instanceDir=\"core0\" \/>\n<core name=\"${_MD5H}.${Dom}.${_HM_U}\" instanceDir=\"${_HM_U}.${Dom}\" \/>\n/g" ${_SOLR_BASE}/solr.xml
      wait
      sed -i "/^$/d" ${_SOLR_BASE}/solr.xml &> /dev/null
      wait
      update_solr "${1}" "${2}"
      echo "New Solr with ${1} for ${2} added"
    fi
  fi
}

delete_solr() {
  # ${1} is solr base dir
  # ${2} is solr core path
  if [ "${1}" = "/opt/solr4" ]; then
    _JTTY_FAM="etty9"
    _SOLR_BASE="/opt/solr4"
  elif [ "${1}" = "/opt/solr7" ]; then
    _JTTY_FAM="etty97"
    _SOLR_BASE="/opt/solr7"
  fi
  if [ ! -z "${2}" ] \
    && [ -e "/var/xdrago/conf/solr" ] \
    && [ -e "${2}/conf" ]; then
    sed -i "s/.*instanceDir=\"${_HM_U}.${Dom}\".*//g" ${_SOLR_BASE}/solr.xml
    wait
    sed -i "/^$/d" ${_SOLR_BASE}/solr.xml &> /dev/null
    wait
    rm -rf ${2}
    rm -f ${Dir}/solr.php
    if [ -e "/etc/default/j${_JTTY_FAM}" ] && [ -e "/etc/init.d/j${_JTTY_FAM}" ]; then
      kill -9 $(ps aux | grep '[j]${_JTTY_FAM}' | awk '{print $2}') &> /dev/null
      service j${_JTTY_FAM} start &> /dev/null
    fi
    echo "Deleted Solr core in ${2}"
  fi
}

check_solr() {
  # ${1} is module
  # ${2} is solr core path
  if [ ! -z "${1}" ] && [ ! -z "${2}" ] && [ -e "/var/xdrago/conf/solr" ]; then
    echo "Checking Solr with ${1} for ${2}"
    if [ ! -e "${2}" ]; then
      add_solr "${1}" "${2}"
    else
      update_solr "${1}" "${2}"
    fi
  fi
}

setup_solr() {

  if [ -e "${Plr}/modules/o_contrib" ]; then
    _SOLR_BASE="/opt/solr4"
    _SOLR_DIR="${_SOLR_BASE}/${_HM_U}.${Dom}"
  elif [ -e "${Plr}/modules/o_contrib_seven" ]; then
    _SOLR_BASE="/opt/solr7"
    _SOLR_DIR="${_SOLR_BASE}/${_HM_U}.${Dom}"
  elif [ -e "${Plr}/modules/o_contrib_eight" ]; then
    _SOLR_BASE="/opt/solr7"
    _SOLR_DIR="${_SOLR_BASE}/${_HM_U}.${Dom}"
  fi

  if [ -e "/data/conf/default.boa_site_control.ini" ] \
    && [ ! -e "${_DIR_CTRL_F}" ]; then
    cp -af /data/conf/default.boa_site_control.ini ${_DIR_CTRL_F} &> /dev/null
    chown ${_HM_U}:users ${_DIR_CTRL_F} &> /dev/null
    chmod 0664 ${_DIR_CTRL_F} &> /dev/null
  fi

  ###
  ### Support for solr_custom_config directive
  ###
  if [ -e "${_DIR_CTRL_F}" ]; then
    _SLR_CM_CFG_P=$(grep "solr_custom_config" ${_DIR_CTRL_F} 2>&1)
    if [[ "${_SLR_CM_CFG_P}" =~ "solr_custom_config" ]]; then
      _DO_NOTHING=YES
    else
      echo ";solr_custom_config = NO" >> ${_DIR_CTRL_F}
    fi
    _SLR_CM_CFG_RT=NO
    _SOLR_PROTECT_CTRL="${_SOLR_DIR}/conf/.protected.conf"
    _SLR_CM_CFG_T=$(grep "^solr_custom_config = YES" ${_DIR_CTRL_F} 2>&1)
    if [[ "${_SLR_CM_CFG_T}" =~ "solr_custom_config = YES" ]]; then
      _SLR_CM_CFG_RT=YES
      if [ ! -e "${_SOLR_PROTECT_CTRL}" ]; then
        touch ${_SOLR_PROTECT_CTRL}
      fi
      echo "Solr config for ${_SOLR_DIR} is protected"
    else
      if [ -e "${_SOLR_PROTECT_CTRL}" ]; then
        rm -f ${_SOLR_PROTECT_CTRL}
      fi
    fi
  fi
  ###
  ### Support for solr_integration_module directive
  ###
  if [ -e "${_DIR_CTRL_F}" ]; then
    _SOLR_MODULE="your_module_name_here"
    _SOLR_IM_PT=$(grep "solr_integration_module" ${_DIR_CTRL_F} 2>&1)
    if [[ "${_SOLR_IM_PT}" =~ "solr_integration_module" ]]; then
      _DO_NOTHING=YES
    else
      echo ";solr_integration_module = your_module_name_here" >> ${_DIR_CTRL_F}
    fi
    _ASOLR_T=$(grep "^solr_integration_module = apachesolr" \
      ${_DIR_CTRL_F} 2>&1)
    if [[ "${_ASOLR_T}" =~ "apachesolr" ]]; then
      _SOLR_MODULE="apachesolr"
    fi
    _SAPI_SOLR_T=$(grep "^solr_integration_module = search_api_solr" \
      ${_DIR_CTRL_F} 2>&1)
    if [[ "${_SAPI_SOLR_T}" =~ "search_api_solr" ]]; then
      _SOLR_MODULE="search_api_solr"
    fi
    if [ "${_SOLR_MODULE}" = "search_api_solr" ] || [ "${_SOLR_MODULE}" = "apachesolr" ]; then
      check_solr "${_SOLR_MODULE}" "${_SOLR_DIR}"
    else
      if [ -e "${_SOLR_DIR}" ]; then
        delete_solr "${_SOLR_BASE}" "${_SOLR_DIR}"
      fi
    fi
  fi
  ###
  ### Support for solr_update_config directive
  ###
  if [ -e "${_DIR_CTRL_F}" ]; then
    _SOLR_UP_CFG_PT=$(grep "solr_update_config" ${_DIR_CTRL_F} 2>&1)
    if [[ "${_SOLR_UP_CFG_PT}" =~ "solr_update_config" ]]; then
      _DO_NOTHING=YES
    else
      echo ";solr_update_config = NO" >> ${_DIR_CTRL_F}
    fi
    _SOLR_UP_CFG_TT=$(grep "^solr_update_config = YES" ${_DIR_CTRL_F} 2>&1)
    if [[ "${_SOLR_UP_CFG_TT}" =~ "solr_update_config = YES" ]]; then
      if [ "${_SLR_CM_CFG_RT}" = "NO" ] \
        && [ ! -e "${_SOLR_PROTECT_CTRL}" ]; then
        update_solr "${_SOLR_MODULE}" "${_SOLR_DIR}"
      fi
    fi
  fi
}

proceed_solr() {
  if [ ! -z "${Dan}" ] \
    && [ "${Dan}" != "hostmaster" ]; then
    setup_solr
  fi
}

check_sites_list() {
  for Site in `find ${User}/config/server_master/nginx/vhost.d \
    -maxdepth 1 -mindepth 1 -type f | sort`; do
    _MOMENT=$(date +%y%m%d-%H%M 2>&1)
    echo ${_MOMENT} Start Counting Site $Site
    Dom=""
    Dan=""
    Plr=""
    Plx=""
    Dom=$(echo $Site | cut -d'/' -f9 | awk '{ print $1}' 2>&1)
    if [ -e "${User}/config/server_master/nginx/vhost.d/${Dom}" ]; then
      Plx=$(cat ${User}/config/server_master/nginx/vhost.d/${Dom} \
        | grep "root " \
        | cut -d: -f2 \
        | awk '{ print $2}' \
        | sed "s/[\;]//g" 2>&1)
      if [[ "$Plx" =~ "aegir/distro" ]]; then
        Dan="hostmaster"
      else
        Dan="${Dom}"
      fi
    fi
    _STATUS_DISABLED=NO
    _STATUS_TEST=$(grep "Do not reveal Aegir front-end URL here" \
      ${User}/config/server_master/nginx/vhost.d/${Dom} 2>&1)
    if [[ "${_STATUS_TEST}" =~ "Do not reveal Aegir front-end URL here" ]]; then
      _STATUS_DISABLED=YES
      echo "${Dom} site is DISABLED"
    fi
    if [ -e "${User}/.drush/${Dan}.alias.drushrc.php" ] \
      && [ "${_STATUS_DISABLED}" = "NO" ]; then
      echo "Dom is ${Dom}"
      Dir=$(cat ${User}/.drush/${Dan}.alias.drushrc.php \
        | grep "site_path'" \
        | cut -d: -f2 \
        | awk '{ print $3}' \
        | sed "s/[\,']//g" 2>&1)
      _DIR_CTRL_F="${Dir}/modules/boa_site_control.ini"
      Plr=$(cat ${User}/.drush/${Dan}.alias.drushrc.php \
        | grep "root'" \
        | cut -d: -f2 \
        | awk '{ print $3}' \
        | sed "s/[\,']//g" 2>&1)
      _PLR_CTRL_F="${Plr}/sites/all/modules/boa_platform_control.ini"
      proceed_solr
    fi
  done
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
  echo ${_CPU_NR} > /data/all/cpuinfo
  chmod 644 /data/all/cpuinfo &> /dev/null
}

load_control() {
  if [ -e "/root/.barracuda.cnf" ]; then
    source /root/.barracuda.cnf
    _CPU_MAX_RATIO=${_CPU_MAX_RATIO//[^0-9]/}
  fi
  if [ -z "${_CPU_MAX_RATIO}" ]; then
    _CPU_MAX_RATIO=6
  fi
  _O_LOAD=$(awk '{print $1*100}' /proc/loadavg 2>&1)
  _O_LOAD=$(( _O_LOAD / _CPU_NR ))
  _O_LOAD_MAX=$(( 100 * _CPU_MAX_RATIO ))
}

start_up() {
  for User in `find /data/disk/ -maxdepth 1 -mindepth 1 | sort`; do
    count_cpu
    load_control
    if [ -e "${User}/config/server_master/nginx/vhost.d" ] \
      && [ ! -e "${User}/log/CANCELLED" ]; then
      if [ "${_O_LOAD}" -lt "${_O_LOAD_MAX}" ]; then
        _HM_U=$(echo ${User} | cut -d'/' -f4 | awk '{ print $1}' 2>&1)
        _THIS_HM_SITE=$(cat ${User}/.drush/hostmaster.alias.drushrc.php \
          | grep "site_path'" \
          | cut -d: -f2 \
          | awk '{ print $3}' \
          | sed "s/[\,']//g" 2>&1)
        echo "load is ${_O_LOAD} while maxload is ${_O_LOAD_MAX}"
        echo "User ${User}"
        mkdir -p ${User}/log/ctrl
        if [ -e "/root/.${_HM_U}.octopus.cnf" ]; then
          source /root/.${_HM_U}.octopus.cnf
          _MY_EMAIL=${_MY_EMAIL//\\\@/\@}
        fi
        check_sites_list
      fi
    fi
  done
}

start_up
exit 0

