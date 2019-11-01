#----------------------------------------------------------------------------
#
#  $Id$
#
#  Copyright (C) 2006-2010  Florian Pose, Ingenieurgemeinschaft IgH
#
#  This file is part of the IgH EtherCAT Master.
#
#  The IgH EtherCAT Master is free software; you can redistribute it and/or
#  modify it under the terms of the GNU General Public License version 2, as
#  published by the Free Software Foundation.
#
#  The IgH EtherCAT Master is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
#  Public License for more details.
#
#  You should have received a copy of the GNU General Public License along
#  with the IgH EtherCAT Master; if not, write to the Free Software
#  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
#
#  ---
#
#  The license mentioned above concerns the source code only. Using the
#  EtherCAT technology and brand is only permitted in compliance with the
#  industrial property and similar rights of Beckhoff Automation GmbH.
#
#  vim: tw=78
#
#----------------------------------------------------------------------------

Name: ethercat
Version: 1.5.2
Release: 1

License: GPL
URL: http://etherlab.org/en/ethercat

Provides: ethercat
BuildRoot: %{_tmppath}/%{name}-%{version}

#BuildRequires: %kernel_module_package_buildreqs

#----------------------------------------------------------------------------
# Main Package
#----------------------------------------------------------------------------

Summary: IgH EtherCAT Master
Group: EtherLab

%description
This is an open-source EtherCAT master implementation for $__KERNELRELEASE . See the
FEATURES file for a list of features. For more information, see
http://etherlab.org/en/ethercat.

%kernel_module_package

#----------------------------------------------------------------------------
# Development package
#----------------------------------------------------------------------------

%package devel

Summary: Development files for applications that use the EtherCAT master.
Group: EtherLab

%description devel
This is an open-source EtherCAT master implementation for $__KERNELRELEASE . See the
FEATURES file for a list of features. For more information, see
http://etherlab.org/en/ethercat.

#----------------------------------------------------------------------------

%build
#%bootstrap
%configure --enable-sii-assign --disable-8139too --disable-eoe --enable-igb --libdir=/usr/xenomai/lib/ \
    --with-xenomai-dir=/usr/xenomai --with-xenomai-config=/usr/xenomai/bin/xeno-config \
    --with-linux-dir=<kernel output dir> --with-devices=8 --enable-rtdm \
    --enable-hrtimer --enable-cycles
make all modules %{?_smp_mflags}
mkdir -p ${RPM_BUILD_ROOT}/usr/bin

%install
make DESTDIR=${RPM_BUILD_ROOT} install
make INSTALL_MOD_PATH=${RPM_BUILD_ROOT} modules_install

%clean
rm -rf ${RPM_BUILD_ROOT}

%files
%defattr(-,root,root)
%doc AUTHORS
%doc COPYING
%doc COPYING.LESSER
%doc ChangeLog
%doc FEATURES
%doc INSTALL
%doc NEWS
%doc README
%doc README.EoE
/etc/init.d/ethercat
/etc/sysconfig/ethercat
/usr/bin/ethercat
/usr/xenomai/lib/libethercat.so*
/usr/xenomai/lib/libethercat_rtdm.so*
/etc/ethercat.conf
/usr/sbin/ethercatctl
/usr/xenomai/lib/systemd/system/ethercat.service
/lib/modules/*

%files devel
%defattr(-,root,root)
/usr/include/*.h
/usr/xenomai/lib/libethercat.a
/usr/xenomai/lib/libethercat_rtdm.a
/usr/xenomai/lib/libethercat.la
/usr/xenomai/lib/libethercat_rtdm.la

#----------------------------------------------------------------------------
