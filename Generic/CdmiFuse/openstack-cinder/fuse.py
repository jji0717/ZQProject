"""
Cinder Driver for Linux servers running FUSE.
Typical location:
    /usr/lib/python2.6/site-packages/cinder/volume/drivers/fuse.py
Related Configuration Files:
    /usr/lib/python2.6/site-packages/cinder/volume/manager.py
    /usr/lib/python2.6/site-packages/cinder/volume/driver.py
    /etc/tgt/targets.conf
    /etc/cinder/cinder.conf

"""
import math
import os
import socket
import struct
import fcntl

from oslo.config import cfg

from cinder.brick import exception as brick_exception
from cinder.brick.local_dev import lvm as lvm
from cinder import exception
from cinder.image import image_utils
from cinder.openstack.common import fileutils
from cinder.openstack.common import log as logging
from cinder.openstack.common import processutils
from cinder import units
from cinder import utils
from cinder.volume import driver
from cinder.volume import utils as volutils

LOG = logging.getLogger(__name__)

volume_opts = [
    cfg.StrOpt('volumes_home',
               default='/mnt/fuse/',
               help='the home directory where contains volume image files'),
]

CONF = cfg.CONF
CONF.register_opts(volume_opts)

class FuseVolumeDriver(driver.VolumeDriver):
    """Executes commands relating to Volumes."""

    VERSION = '1.0.0'

    def __init__(self, *args, **kwargs):
        LOG.debug(_("FuseVolumeDriver()__init__()"))
        super(FuseVolumeDriver, self).__init__(*args, **kwargs)
        self.configuration.append_config_values(volume_opts)
        # self.hostname1 = 'localhost' # socket.gethostname()
        self.hostname = socket.gethostname()
        self.backend_name =\
            self.configuration.safe_get('volume_backend_name') or 'FUSE'
        self.protocol = 'local'
        LOG.debug(_("FuseVolumeDriver()__init__(), backend_name:%s"), self.backend_name)
    def set_execute(self, execute):
        LOG.debug(_("FuseVolumeDriver()set_execute(),%s"), execute)
        self._execute = execute

    def check_for_setup_error(self):
        """Verify that requirements are in place to use FUSE driver."""
        LOG.debug(_("FuseVolumeDriver::check_for_setup_error()"))
        pass
        '''
        if self.vg is None:
            root_helper = utils.get_root_helper()
            try:
                self.vg = lvm.LVM(self.configuration.volume_group,
                                  root_helper,
                                  lvm_type=self.configuration.lvm_type,
                                  executor=self._execute)
            except brick_exception.VolumeGroupNotFound:
                message = ("Volume Group %s does not exist" %
                           self.configuration.volume_group)
                raise exception.VolumeBackendAPIException(data=message)

        vg_list = volutils.get_all_volume_groups(
            self.configuration.volume_group)
        vg_dict = \
            (vg for vg in vg_list if vg['name'] == self.vg.vg_name).next()
        if vg_dict is None:
            message = ("Volume Group %s does not exist" %
                       self.configuration.volume_group)
            raise exception.VolumeBackendAPIException(data=message)

        if self.configuration.lvm_type == 'thin':
            # Specific checks for using Thin provisioned LV's
            if not volutils.supports_thin_provisioning():
                message = ("Thin provisioning not supported "
                           "on this version of LVM.")
                raise exception.VolumeBackendAPIException(data=message)

            pool_name = "%s-pool" % self.configuration.volume_group
            if self.vg.get_volume(pool_name) is None:
                try:
                    self.vg.create_thin_pool(pool_name)
                except processutils.ProcessExecutionError as exc:
                    exception_message = ("Failed to create thin pool, "
                                         "error message was: %s"
                                         % exc.stderr)
                    raise exception.VolumeBackendAPIException(
                        data=exception_message)
        '''
    
    def _sizestr(self, size_in_g):
        if int(size_in_g) == 0:
            return '100m'
        return '%sg' % size_in_g
        LOG.debug(_("FuseVolumeDriver()_sizestr() %sg"), size_in_g)
    
    def local_path(self, volume):
        LOG.debug(_("FuseVolumeDriver()local_path() %s/%s"),self.configuration.volumes_home, volume)
        return "%s/%s" % (self.configuration.volumes_home, volume)

    def _create_volume(self, name, size):
        cmd ="truncate --size=%s %s" % (size, self.local_path(name));
        LOG.debug(_("FuseVolumeDriver::_create_volume() executing: %s"), cmd)
        os.system(cmd)

    def create_volume(self, volume):
        """Creates a volume under FUSE mount home"""
        LOG.debug(_("FuseVolumeDriver::create_volume() v:%s"), volume)
        self._create_volume(volume['name'], self._sizestr(volume['size']))
        
    def _delete_volume(self, volume, is_snapshot=False):
        """Deletes a volume."""
        cmd ="rm -f %s" % (self.local_path(volume['name']));
        LOG.debug(_("FuseVolumeDriver::_delete_volume() executing: %s"), cmd)
        os.system(cmd)

    def delete_volume(self, volume):
        """Deletes a volume."""

        # NOTE(jdg):  We don't need to explicitly call
        # remove export here because we already did it
        # in the manager before we got here.

        if self._volume_not_present(volume['name']):
            # If the volume isn't present, then don't attempt to delete
            return True

        #if self.vg.lv_has_snapshot(volume['name']):
        #    LOG.error(_('Unabled to delete due to existing snapshot for volume: %s') % volume['name'])
        #    raise exception.VolumeIsBusy(volume_name=volume['name'])

        self._delete_volume(volume)
    
    def _volume_not_present(self, volume_name):
        return not os.path.exists(self.local_path(volume_name))
        
    def _offload_clone_volume(self, srcVolumeName, destVolumeName, fastClone=False):
        CLONE_CMD = 1107584257
        # srcFile = self.local_path(srcVolumeName)
        # destFile = self.local_path(destVolumeName)
        if fastClone:
       	   clone =1
        else:
           clone = 0
        clone_params = struct.pack("@HH512s", 0, clone, destVolumeName)  # build up the param struct
        
        cwd = os.getcwd();
        os.chdir(self.local_path(""))
        f = open(srcVolumeName, "r")
        if not f:
            os.chdir(cwd)
            raise exception.VolumeBackendAPIException("can't open specified file: %s" %(srcFile) )
        
        rc = fcntl.ioctl(f.fileno(), CLONE_CMD, clone_params)
        f.close()
        os.chdir(cwd);
        return rc

    def create_snapshot(self, snapshot):
        """Creates a snapshot."""
        
        #perform
        LOG.debug(_("FuseVolumeDriver::create_snapshot(): %s %s"), snapshot['volume_name'],snapshot['name'] )
        snapshotname = str( snapshot['name'])
        snapshotVolumeName=str(snapshot['volume_name'])
        self._offload_clone_volume(snapshotVolumeName, snapshotname)

    def delete_snapshot(self, snapshot):
        """Deletes a snapshot."""
        if self._volume_not_present(self._escape_snapshot(snapshot['name'])):
            # If the snapshot isn't present, then don't attempt to delete
            LOG.warning(_("snapshot: %s not found, "
                          "skipping delete operations") % snapshot['name'])
            return True

        # TODO(yamahata): zeroing out the whole snapshot triggers COW.
        # it's quite slow.
        self._delete_volume(snapshot, is_snapshot=True)
        
    def _escape_snapshot(self, snapshot_name):
        # Linux LVM reserves name that starts with snapshot, so that
        # such volume name can't be created. Mangle it.
        LOG.debug(_("FuseVolumeDriver::_escape_snapshot() %s"), snapshot_name)
        if not snapshot_name.startswith('snapshot'):
            return snapshot_name
        return '_' + snapshot_name

    def create_volume_from_snapshot(self, volume, snapshot):
        """Creates a volume from a snapshot."""
        LOG.debug(_("FuseVolumeDriver::create_volume_from_snapshot() v:%s"), volume)
        self._offload_clone_volume(snapshot['name'], snapshot['volume_name'])

    def copy_image_to_volume(self, context, volume, image_service, image_id):
        """Fetch the image from image_service and write it to the volume."""
        LOG.debug(_("FuseVolumeDriver::copy_image_to_volume() v:%s"), volume)
        image_utils.fetch_to_raw(context,
                                 image_service,
                                 image_id,
                                 self.local_path(volume),
                                 self.configuration.volume_dd_blocksize,
                                 size=volume['size'])

    def copy_volume_to_image(self, context, volume, image_service, image_meta):
        """Copy the volume to the specified image."""
        LOG.debug(_("FuseVolumeDriver::copy_volume_to_image() v:%s"), volume)
        image_utils.upload_volume(context,
                                  image_service,
                                  image_meta,
                                  self.local_path(volume))

    def create_cloned_volume(self, volume, src_vref):
        """Creates a clone of the specified volume."""

        LOG.info(_('Creating clone of volume: %s') % src_vref['id'])
        volume_name = src_vref['name']
        temp_id = 'tmp-snap-%s' % volume['id']
        temp_snapshot = {'volume_name': volume_name,
                         'size': src_vref['size'],
                         'volume_size': src_vref['size'],
                         'name': 'clone-snap-%s' % volume['id'],
                         'id': temp_id}

        self.create_snapshot(temp_snapshot)
        self._create_volume(volume['name'], self._sizestr(volume['size']))

       # copy_volume expects sizes in MiB, we store integer GiB
       # be sure to convert before passing in
        try:
            volutils.copy_volume(
                self.local_path(temp_snapshot),
                self.local_path(volume),
                src_vref['size'] * units.KiB,
                self.configuration.volume_dd_blocksize,
                execute=self._execute)
        finally:
            self.delete_snapshot(temp_snapshot)

    def clone_image(self, volume, image_location, image_id, image_meta):
        LOG.debug(_("FuseVolumeDriver::clone_image()"))
        return None, False

    def backup_volume(self, context, backup, backup_service):
        """Create a new backup from an existing volume."""
        LOG.debug(_("FuseVolumeDriver::backup_volume()"))
        volume = self.db.volume_get(context, backup['volume_id'])
        volume_path = self.local_path(volume)
        with utils.temporary_chown(volume_path):
            with fileutils.file_open(volume_path) as volume_file:
                backup_service.backup(backup, volume_file)

    def restore_backup(self, context, backup, volume, backup_service):
        """Restore an existing backup to a new or existing volume."""
        LOG.debug(_("FuseVolumeDriver::restore_backup() volume=%s") % volume)
        volume_path = self.local_path(volume)
        with utils.temporary_chown(volume_path):
            with fileutils.file_open(volume_path, 'wb') as volume_file:
                backup_service.restore(backup, volume['id'], volume_file)

    def _update_volume_stats(self):
        """Retrieve stats info of volume image"""

        LOG.debug(_("FuseVolumeDriver updating volume stats: backend_name[%s] self:%s") % (self.backend_name, dir(self)))
        data = {}
        data["volume_backend_name"] = self.backend_name
        data["vendor_name"] = 'XOR Media'
        data["driver_version"] = self.VERSION
        data["storage_protocol"] = self.protocol

        data['total_capacity_gb'] = 200
        data['free_capacity_gb'] = 150
        data['reserved_percentage'] = self.configuration.reserved_percentage
        data['QoS_support'] = False
        data['location_info'] =\
            ('FuseVolumeDriver:%(hostname)s:' %
             {'hostname': self.host})
        '''
        TODO
        # Note(zhiteng): These information are driver/backend specific,
        # each driver may define these values in its own config options
        # or fetch from driver specific configuration file.

        if self.configuration.lvm_mirrors > 0:
            data['total_capacity_gb'] =\
                self.vg.vg_mirror_size(self.configuration.lvm_mirrors)
            data['free_capacity_gb'] =\
                self.vg.vg_mirror_free_space(self.configuration.lvm_mirrors)
        elif self.configuration.lvm_type == 'thin':
            data['total_capacity_gb'] = self.vg.vg_thin_pool_size
            data['free_capacity_gb'] = self.vg.vg_thin_pool_free_space
        else:
            data['total_capacity_gb'] = self.vg.vg_size
            data['free_capacity_gb'] = self.vg.vg_free_space
        data['reserved_percentage'] = self.configuration.reserved_percentage
        data['QoS_support'] = False
        data['location_info'] =\
            ('LVMVolumeDriver:%(hostname)s:%(vg)s'
             ':%(lvm_type)s:%(lvm_mirrors)s' %
             {'hostname': self.hostname,
              'vg': self.configuration.volume_group,
              'lvm_type': self.configuration.lvm_type,
              'lvm_mirrors': self.configuration.lvm_mirrors})

        '''
        self._stats = data
        LOG.debug(_("FuseVolumeDriver volume stats updated: %s") % self._stats)

    def get_volume_stats(self, refresh=False):
        """Get volume status.
        If 'refresh' is True, run update the stats first.
        """
        LOG.debug(_("FuseVolumeDriver::get_volume_stats() refresh=%s") % refresh)

        if refresh:
            self._update_volume_stats()

        return self._stats

    def extend_volume(self, volume, new_size):
        """Extend an existing volume's size."""
        LOG.debug(_("FuseVolumeDriver::extend_volume() volume=%s to newsize=%d") % (volume, new_size))
        cmd ="truncate --no-create --size=%s %s" % (size, self.local_path(name));

    def manage_existing(self, volume, existing_ref):
        LOG.debug(_("FuseVolumeDriver::manage_existing() volume=%s") % volume)
        """Manages an existing volume.
        Renames the LV to match the expected name for the volume.
        Error checking done by manage_existing_get_size is not repeated.
        """
        ''' TODO
        lv_name = existing_ref['lv_name']
        lv = self.vg.get_volume(lv_name)

        # Attempt to rename the LV to match the OpenStack internal name.
        try:
            self.vg.rename_volume(lv_name, volume['name'])
        except processutils.ProcessExecutionError as exc:
            exception_message = (_("Failed to rename logical volume %(name)s, "
                                   "error message was: %(err_msg)s")
                                 % {'name': lv_name,
                                    'err_msg': exc.stderr})
            raise exception.VolumeBackendAPIException(
                data=exception_message)
        '''
        msg = _("Manage existing volume not implemented.")
        raise NotImplementedError(msg)

    def manage_existing_get_size(self, volume, existing_ref):
        LOG.debug(_("FuseVolumeDriver::manage_existing_get_size() volume=%s") % volume)
        """Return size of volume to be managed by manage_existing.

        When calculating the size, round up to the next GB.
        """
        msg = _("Manage existing volume not implemented.")
        raise NotImplementedError(msg)

class FuseISCSIDriver(FuseVolumeDriver, driver.ISCSIDriver):
    """Executes commands relating to ISCSI volumes.

    We make use of model provider properties as follows:

    ``provider_location``
      if present, contains the iSCSI target information in the same
      format as an ietadm discovery
      i.e. '<ip>:<port>,<portal> <target IQN>'

    ``provider_auth``
      if present, contains a space-separated triple:
      '<auth method> <auth username> <auth password>'.
      `CHAP` is the only auth_method in use at the moment.
    """

    def __init__(self, *args, **kwargs):
        self.db = kwargs.get('db')
        self.target_helper = self.get_target_helper(self.db)
        super(FuseISCSIDriver, self).__init__(*args, **kwargs)
        self.backend_name =\
            self.configuration.safe_get('volume_backend_name') or 'FUSE_iSCSI'
        self.protocol = 'iSCSI'
        LOG.debug(_("FuseISCSIDriver()__init__"))

    def set_execute(self, execute):
        LOG.debug(_("FuseISCSIDriver::set_execute(),%s"), execute)
        super(FuseISCSIDriver, self).set_execute(execute)
        if self.target_helper is not None:
            LOG.debug(_("FuseISCSIDriver::target_helper.set_execute(),%s"), execute)
            self.target_helper.set_execute(execute)

    def _create_target(self, iscsi_name, iscsi_target,
                       volume_path, chap_auth, lun=0,
                       check_exit_code=False, old_name=None):
        # NOTE(jdg): tgt driver has an issue where with a lot of activity
        # (or sometimes just randomly) it will get *confused* and attempt
        # to reuse a target ID, resulting in a target already exists error
        # Typically a simple retry will address this

        # For now we have this while loop, might be useful in the
        # future to throw a retry decorator in common or utils
        LOG.debug(_("FUSEISCSIDriver()_create_target()"))
        attempts = 2
        while attempts > 0:
            attempts -= 1
            try:
                # NOTE(jdg): For TgtAdm case iscsi_name is all we need
                # should clean this all up at some point in the future
                tid = self.target_helper.create_iscsi_target(
                    iscsi_name,
                    iscsi_target,
                    0,
                    volume_path,
                    chap_auth,
                    check_exit_code=check_exit_code,
                    old_name=old_name)
                break

            except brick_exception.ISCSITargetCreateFailed:
                if attempts == 0:
                    raise
                else:
                    LOG.warning(_('Error creating iSCSI target, retrying '
                                  'creation for target: %s') % iscsi_name)
        return tid

    def ensure_export(self, context, volume):
        LOG.debug(_("FuseISCSIDriver::ensure_export() volume=%s") % volume)
        volume_name = volume['name']
        iscsi_name = "%s%s" % (self.configuration.iscsi_target_prefix,
                               volume_name)
        volume_path = self.local_path(volume_name)
        
        # NOTE(jdg): For TgtAdm case iscsi_name is the ONLY param we need
        # should clean this all up at some point in the future
        model_update = self.target_helper.ensure_export(context, volume,
                                                        iscsi_name,
                                                        volume_path)
        if model_update:
            self.db.volume_update(context, volume['id'], model_update)

    def create_export(self, context, volume):
        LOG.debug(_("FuseISCSIDriver::create_export() volume=%s") % volume)
        return self._create_export(context, volume)
    
    def _create_export(self, context, volume, vg=None):
        """Creates an export for a logical volume."""
        volume_path = self.local_path(volume['name'])

        data = self.target_helper.create_export(context, volume, volume_path)
        return {
            'provider_location': data['location'],
            'provider_auth': data['auth'],
        }

    def remove_export(self, context, volume):
        LOG.debug(_("FuseISCSIDriver::remove_export() volume=%s") % volume)
        self.target_helper.remove_export(context, volume)

    def migrate_volume(self, ctxt, volume, host, thin=False, mirror_count=0):
        """Optimize the migration if the destination is on the same server.

        If the specified host is another back-end on the same server, and
        the volume is not attached, we can do the migration locally without
        going through iSCSI.
        """

        false_ret = (False, None)
        if volume['status'] != 'available':
            return false_ret
        if 'location_info' not in host['capabilities']:
            return false_ret
        info = host['capabilities']['location_info']
        
        LOG.debug(_("FuseISCSIDriver::migrate_volume() info=%s") % info)
        return false_ret
        ''' TODO
        try:
            (dest_type, dest_hostname, dest_vg, lvm_type, lvm_mirrors) = info.split(':')
            lvm_mirrors = int(lvm_mirrors)
        except ValueError:
            return false_ret
        if (dest_type != 'LVMVolumeDriver' or dest_hostname != self.hostname):
            return false_ret

        if dest_vg != self.vg.vg_name:
            vg_list = volutils.get_all_volume_groups()
            try:
                (vg for vg in vg_list if vg['name'] == dest_vg).next()
            except StopIteration:
                message = ("Destination Volume Group %s does not exist" %
                           dest_vg)
                LOG.error(_('%s'), message)
                return false_ret

            helper = utils.get_root_helper()
            dest_vg_ref = lvm.LVM(dest_vg, helper,
                                  lvm_type=lvm_type,
                                  executor=self._execute)
            self.remove_export(ctxt, volume)
            self._create_volume(volume['name'],
                                self._sizestr(volume['size']),
                                lvm_type,
                                lvm_mirrors,
                                dest_vg_ref)

        volutils.copy_volume(self.local_path(volume),
                             self.local_path(volume, vg=dest_vg),
                             volume['size'],
                             self.configuration.volume_dd_blocksize,
                             execute=self._execute)
        self._delete_volume(volume)
        model_update = self._create_export(ctxt, volume, vg=dest_vg)

        return (True, model_update)
        '''
        
    def _iscsi_location(self, ip, target, iqn, lun=None):
        LOG.debug(_("FuseISCSIDriver()_iscsi_location"))
        return "%s:%s,%s %s %s" % (ip, self.configuration.iscsi_port,
                                   target, iqn, lun)

    def _iscsi_authentication(self, chap, name, password):
        LOG.debug(_("FuseISCSIDriver()_iscsi_location"))
        return "%s %s %s" % (chap, name, password)

    
