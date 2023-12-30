Cinder Driver for Linux servers running FUSE.

base openstack version:
python-cinder-2014.1-4.el6.noarch
python-cinderclient-1.0.9-1.el6.noarch
openstack-cinder-2014.1-4.el6.noarch

Instructions:
1) drop this fuse.py under /usr/lib/python2.6/site-packages/cinder/volume/drivers
2) edit /usr/lib/python2.6/site-packages/cinder/volume/manager.py
   MAPPING = {
    ...
    'cinder.volume.driver.ISCSIDriver': 'cinder.volume.drivers.fuse.FuseISCSIDriver',
    ...
    }
3) edit /usr/lib/python2.6/site-packages/cinder/volume/driver.py
class ISCSIDriver(VolumeDriver):
    ...
    def _get_iscsi_properties(self, volume):
        ...
        try:
            properties['target_lun'] = int(results[2])
        except (IndexError, ValueError):
            if (self.configuration.volume_driver in
                    ['cinder.volume.drivers.lvm.LVMISCSIDriver',
                     'cinder.volume.drivers.lvm.ThinLVMVolumeDriver',
                     'cinder.volume.drivers.fuse.FuseISCSIDriver'] and # << insert fuse.FuseISCSIDriver in the list
                    self.configuration.iscsi_helper == 'tgtadm'):
                properties['target_lun'] = 1
            else:
                properties['target_lun'] = 0
        ...
4) make sure /etc/tgt/targets.conf includes line:
    include /etc/cinder/volumes/*

5) assume CdmiFUSE's mount point is /mnt/fuse and the volumes are put under /mnt/fuse/volumes/, edit /etc/cinder/cinder.conf
[DEFAULT]
volume_driver=cinder.volume.drivers.fuse.FuseISCSIDriver
volumes_home=/mnt/fuse/volumes

6) restart openstack-cinder-volume and tgtd


export volume:
1) edit /usr/lib/python2.6/site-packages/cinderclient/v1/shell.py
add export and delete-export command:
@utils.arg('volume',
           metavar='<volume>', nargs='+',
           help='Name or ID of the volume(s) to export.')
@utils.service_type('volume')
def do_export(cs, args):
    """export a volume(s)."""
    failure_count = 0
    for volume in args.volume:
        try:
            utils.find_volume(cs, volume).export()
        except Exception as e:
            failure_count += 1
            print("export for volume %s failed: %s" % (volume, e))
    if failure_count == len(args.volume):
        raise exceptions.CommandError("Unable to export any of the specified "
                                      "volumes.")

@utils.arg('volume',
           metavar='<volume>', nargs='+',
           help='Name or ID of the volume(s) to delete export.')
@utils.service_type('volume')
def do_delete_export(cs, args):
    """delete export volume(s)."""
    failure_count = 0
    for volume in args.volume:
        try:
            utils.find_volume(cs, volume).delete_export()
        except Exception as e:
            failure_count += 1
            print("delete export for volume %s failed: %s" % (volume, e))
    if failure_count == len(args.volume):
        raise exceptions.CommandError("Unable to delete export volume any of the specified "
                                      "volumes.")
                                     "volumes.")
2) edit /usr/lib/python2.6/site-packages/cinderclient/v1/volumes.py
class Volume(base.Resource):
   def export(self):
        """Set export metadata.
        """
        return self.manager.export(self)
     
    def delete_export(self):
        """Set export metadata.
        """
        return self.manager.delete_export(self)	 

class VolumeManager(base.ManagerWithFind):
    def export(self, volume):
        """
        Set export metadata.

        :param volume: The :class:`Volume` (or its ID)
                       you would like to export.
        """
        return self._action('os-export', volume)
		
    def delete_export(self, volume):
        """
        Set delete_export metadata.

        :param volume: The :class:`Volume` (or its ID)
                       you would like to export.
        """
        return self._action('os-delete-export', volume)

cinder api service node:
1)edit /usr/lib/python2.6/site-packages/cinder/volume/api.py
class API(base.Base):
    @wrap_check_policy
    def export(self, context, volume):
        volume_metadata = self.get_volume_admin_metadata(context.elevated(),
                                                         volume)
        if 'readonly' not in volume_metadata:
            # NOTE(zhiyan): set a default value for read-only flag to metadata.
            self.update_volume_admin_metadata(context.elevated(), volume,
                                              {'readonly': 'False'})
            volume_metadata['readonly'] = 'False'
 
        if volume_metadata['readonly'] == 'True' and mode != 'ro':
           raise exception.InvalidVolumeAttachMode(mode=mode,
                                                    volume_id=volume['id'])

        return self.volume_rpcapi.export_volume(context,volume)

    @wrap_check_policy
    def delete_export(self, context, volume):
        volume_metadata = self.get_volume_admin_metadata(context.elevated(),
                                                         volume)
        if 'readonly' not in volume_metadata:
            # NOTE(zhiyan): set a default value for read-only flag to metadata.
            self.update_volume_admin_metadata(context.elevated(), volume,
                                              {'readonly': 'False'})
            volume_metadata['readonly'] = 'False'
 
        if volume_metadata['readonly'] == 'True' and mode != 'ro':
           raise exception.InvalidVolumeAttachMode(mode=mode,
                                                    volume_id=volume['id'])

        return self.volume_rpcapi.delete_export(context,volume)

2)edit /usr/lib/python2.6/site-packages/cinder/volume/rpcapi.py
class VolumeAPI(object):
    def export_volume(self, ctxt, volume):
        cctxt = self.client.prepare(server=volume['host'])
        return cctxt.call(ctxt, 'export_volume', volume_id=volume['id'])

    def delete_export(self, ctxt, volume):
        cctxt = self.client.prepare(server=volume['host'])
        return cctxt.call(ctxt, 'delete_export', volume_id=volume['id'])

3)edit /usr/lib/python2.6/site-packages/cinder/api/contrib/volume_actions.py
class VolumeActionsController(wsgi.Controller):
    @wsgi.action('os-export')
    def _export(self, req, id, body):
        """Add export metadata."""
        context = req.environ['cinder.context']
        print('_export[%s]', context)
        try:
            volume = self.volume_api.get(context, id)
        except exception.VolumeNotFound as error:
            raise webob.exc.HTTPNotFound(explanation=error.msg)

        self.volume_api.export(context, volume)
        
        return webob.Response(status_int=202)
        
    @wsgi.action('os-delete-export')
    def _delete_export(self, req, id, body):
        """Add delete_export metadata."""
        context = req.environ['cinder.context']
        print('_delete_exprot[%s]', context)
        try:
            volume = self.volume_api.get(context, id)
        except exception.VolumeNotFound as error:
            raise webob.exc.HTTPNotFound(explanation=error.msg)

        self.volume_api.delete_export(context, volume)
        
        return webob.Response(status_int=202)

cinder volume service node:
edit /usr/lib/python2.6/site-packages/cinder/volume/manager.py
class VolumeManager(manager.SchedulerDependentManager):
    def export_volume(self, context, volume_id):
        # check the volume status before exporting
        volume = self.db.volume_get(context, volume_id)
        volume_metadata = self.db.volume_admin_metadata_get(context.elevated(), volume_id)
        if volume['status'] != "available" and volume['status'] != "export" :
            msg = _("status must be available or exporting")
            raise exception.InvalidVolume(reason=msg)

        # TODO(jdg): attach_time column is currently varchar
        # we should update this to a date-time object
        # also consider adding detach_time?
        self._notify_about_volume_usage(context, volume, "export.start")
        self.db.volume_update(context, volume_id, {"status": "exporting"})

        try:
            LOG.debug(_("Volume %s: creating export"), volume_id)
            utils.require_driver_initialized(self.driver)
            model_update = self.driver.create_export(context, volume)
            if model_update:
                volume = self.db.volume_update(context, volume_id,
                                               model_update)
        except exception.CinderException as ex:
            if model_update:
                LOG.exception(_("Failed updating model of volume "
                                "%(volume_id)s with driver provided model "
                                "%(model)s") %
                              {'volume_id': volume['id'],
                               'model': model_update})
                raise exception.ExportFailure(reason=ex)

        except Exception:
            with excutils.save_and_reraise_exception():
                self.db.volume_update(context, volume_id,
                                      {'status': 'available'})
                                         
        self.db.volume_update(context, volume_id,
                              {'status': 'in-use'})
        self._notify_about_volume_usage(context, volume, "export.end")
     
    def delete_export(self, context, volume_id):
        # check the volume status before exporting
        volume = self.db.volume_get(context, volume_id)
        volume_metadata = self.db.volume_admin_metadata_get(context.elevated(), volume_id)
        if volume['status'] != "in-use":
            msg = _("status must be in-use")
            raise exception.InvalidVolume(reason=msg)

        # TODO(jdg): attach_time column is currently varchar
        # we should update this to a date-time object
        # also consider adding detach_time?
        self._notify_about_volume_usage(context, volume, "delete_export.start")
        self.db.volume_update(context, volume_id, {"status": "delete_exporting"})

        try:
            LOG.debug(_("Volume %s: deleting export"), volume_id)
            utils.require_driver_initialized(self.driver)
            model_update = self.driver.remove_export(context, volume)
            if model_update:
                volume = self.db.volume_update(context, volume_id,
                                               model_update)
        except exception.CinderException as ex:
            if model_update:
                LOG.exception(_("Failed updating model of volume "
                                "%(volume_id)s with driver provided model "
                                "%(model)s") %
                              {'volume_id': volume['id'],
                               'model': model_update})
                raise exception.ExportFailure(reason=ex)

        except Exception:
            with excutils.save_and_reraise_exception():
                self.db.volume_update(context, volume_id,
                                      {'status': 'error_delete_exporting'})
                                         
        self.db.volume_update(context, volume_id,
                              {'status': 'available'})
        self._notify_about_volume_usage(context, volume, "delete_export.end")


