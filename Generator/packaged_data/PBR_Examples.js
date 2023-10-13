
  var Module = typeof Module !== 'undefined' ? Module : {};

  if (!Module.expectedDataFileDownloads) {
    Module.expectedDataFileDownloads = 0;
  }

  Module.expectedDataFileDownloads++;
  (function() {
    // Do not attempt to redownload the virtual filesystem data when in a pthread or a Wasm Worker context.
    if (Module['ENVIRONMENT_IS_PTHREAD'] || Module['$ww']) return;
    var loadPackage = function(metadata) {

      var PACKAGE_PATH = '';
      if (typeof window === 'object') {
        PACKAGE_PATH = window['encodeURIComponent'](window.location.pathname.toString().substring(0, window.location.pathname.toString().lastIndexOf('/')) + '/');
      } else if (typeof process === 'undefined' && typeof location !== 'undefined') {
        // web worker
        PACKAGE_PATH = encodeURIComponent(location.pathname.toString().substring(0, location.pathname.toString().lastIndexOf('/')) + '/');
      }
      var PACKAGE_NAME = 'packaged_data/PBR_Examples.data';
      var REMOTE_PACKAGE_BASE = 'PBR_Examples.data';
      if (typeof Module['locateFilePackage'] === 'function' && !Module['locateFile']) {
        Module['locateFile'] = Module['locateFilePackage'];
        err('warning: you defined Module.locateFilePackage, that has been renamed to Module.locateFile (using your locateFilePackage for now)');
      }
      var REMOTE_PACKAGE_NAME = Module['locateFile'] ? Module['locateFile'](REMOTE_PACKAGE_BASE, '') : REMOTE_PACKAGE_BASE;
var REMOTE_PACKAGE_SIZE = metadata['remote_package_size'];

      function fetchRemotePackage(packageName, packageSize, callback, errback) {
        if (typeof process === 'object' && typeof process.versions === 'object' && typeof process.versions.node === 'string') {
          require('fs').readFile(packageName, function(err, contents) {
            if (err) {
              errback(err);
            } else {
              callback(contents.buffer);
            }
          });
          return;
        }
        var xhr = new XMLHttpRequest();
        xhr.open('GET', packageName, true);
        xhr.responseType = 'arraybuffer';
        xhr.onprogress = function(event) {
          var url = packageName;
          var size = packageSize;
          if (event.total) size = event.total;
          if (event.loaded) {
            if (!xhr.addedTotal) {
              xhr.addedTotal = true;
              if (!Module.dataFileDownloads) Module.dataFileDownloads = {};
              Module.dataFileDownloads[url] = {
                loaded: event.loaded,
                total: size
              };
            } else {
              Module.dataFileDownloads[url].loaded = event.loaded;
            }
            var total = 0;
            var loaded = 0;
            var num = 0;
            for (var download in Module.dataFileDownloads) {
            var data = Module.dataFileDownloads[download];
              total += data.total;
              loaded += data.loaded;
              num++;
            }
            total = Math.ceil(total * Module.expectedDataFileDownloads/num);
            if (Module['setStatus']) Module['setStatus'](`Downloading data... (${loaded}/${total})`);
          } else if (!Module.dataFileDownloads) {
            if (Module['setStatus']) Module['setStatus']('Downloading data...');
          }
        };
        xhr.onerror = function(event) {
          throw new Error("NetworkError for: " + packageName);
        }
        xhr.onload = function(event) {
          if (xhr.status == 200 || xhr.status == 304 || xhr.status == 206 || (xhr.status == 0 && xhr.response)) { // file URLs can return 0
            var packageData = xhr.response;
            callback(packageData);
          } else {
            throw new Error(xhr.statusText + " : " + xhr.responseURL);
          }
        };
        xhr.send(null);
      };

      function handleError(error) {
        console.error('package error:', error);
      };

      var fetchedCallback = null;
      var fetched = Module['getPreloadedPackage'] ? Module['getPreloadedPackage'](REMOTE_PACKAGE_NAME, REMOTE_PACKAGE_SIZE) : null;

      if (!fetched) fetchRemotePackage(REMOTE_PACKAGE_NAME, REMOTE_PACKAGE_SIZE, function(data) {
        if (fetchedCallback) {
          fetchedCallback(data);
          fetchedCallback = null;
        } else {
          fetched = data;
        }
      }, handleError);

    function runWithFS() {

      function assert(check, msg) {
        if (!check) throw msg + new Error().stack;
      }
Module['FS_createPath']("/", "Skyboxes", true, true);
Module['FS_createPath']("/Skyboxes", "skybox0", true, true);
Module['FS_createPath']("/Skyboxes", "skybox1", true, true);
Module['FS_createPath']("/Skyboxes", "skybox2", true, true);
Module['FS_createPath']("/", "Textures", true, true);
Module['FS_createPath']("/Textures", "Anisotropic", true, true);
Module['FS_createPath']("/Textures", "Isotropic", true, true);

      /** @constructor */
      function DataRequest(start, end, audio) {
        this.start = start;
        this.end = end;
        this.audio = audio;
      }
      DataRequest.prototype = {
        requests: {},
        open: function(mode, name) {
          this.name = name;
          this.requests[name] = this;
          Module['addRunDependency'](`fp ${this.name}`);
        },
        send: function() {},
        onload: function() {
          var byteArray = this.byteArray.subarray(this.start, this.end);
          this.finish(byteArray);
        },
        finish: function(byteArray) {
          var that = this;
          // canOwn this data in the filesystem, it is a slide into the heap that will never change
          Module['FS_createDataFile'](this.name, null, byteArray, true, true, true);
          Module['removeRunDependency'](`fp ${that.name}`);
          this.requests[this.name] = null;
        }
      };

      var files = metadata['files'];
      for (var i = 0; i < files.length; ++i) {
        new DataRequest(files[i]['start'], files[i]['end'], files[i]['audio'] || 0).open('GET', files[i]['filename']);
      }

      function processPackageData(arrayBuffer) {
        assert(arrayBuffer, 'Loading data file failed.');
        assert(arrayBuffer.constructor.name === ArrayBuffer.name, 'bad input to processPackageData');
        var byteArray = new Uint8Array(arrayBuffer);
        var curr;
        // Reuse the bytearray from the XHR as the source for file reads.
          DataRequest.prototype.byteArray = byteArray;
          var files = metadata['files'];
          for (var i = 0; i < files.length; ++i) {
            DataRequest.prototype.requests[files[i].filename].onload();
          }          Module['removeRunDependency']('datafile_packaged_data/PBR_Examples.data');

      };
      Module['addRunDependency']('datafile_packaged_data/PBR_Examples.data');

      if (!Module.preloadResults) Module.preloadResults = {};

      Module.preloadResults[PACKAGE_NAME] = {fromCache: false};
      if (fetched) {
        processPackageData(fetched);
        fetched = null;
      } else {
        fetchedCallback = processPackageData;
      }

    }
    if (Module['calledRun']) {
      runWithFS();
    } else {
      if (!Module['preRun']) Module['preRun'] = [];
      Module["preRun"].push(runWithFS); // FS is not initialized yet, wait for it
    }

    }
    loadPackage({"files": [{"filename": "/PBR_Parameters.json", "start": 0, "end": 675}, {"filename": "/PBR_Skybox.json", "start": 675, "end": 1131}, {"filename": "/PBR_Skybox_Anisotropy.json", "start": 1131, "end": 1881}, {"filename": "/PBR_Skybox_Texturing.json", "start": 1881, "end": 2597}, {"filename": "/Skyboxes/MapCubeFacesToEquirectangular.md", "start": 2597, "end": 3834}, {"filename": "/Skyboxes/skybox0/back.jpg", "start": 3834, "end": 743902}, {"filename": "/Skyboxes/skybox0/bottom.jpg", "start": 743902, "end": 1024491}, {"filename": "/Skyboxes/skybox0/front.jpg", "start": 1024491, "end": 1497820}, {"filename": "/Skyboxes/skybox0/left.jpg", "start": 1497820, "end": 2099705}, {"filename": "/Skyboxes/skybox0/right.jpg", "start": 2099705, "end": 2637743}, {"filename": "/Skyboxes/skybox0/skybox.jpg", "start": 2637743, "end": 5958173}, {"filename": "/Skyboxes/skybox0/top.jpg", "start": 5958173, "end": 6304312}, {"filename": "/Skyboxes/skybox1/skybox-nx.jpg", "start": 6304312, "end": 6360068}, {"filename": "/Skyboxes/skybox1/skybox-ny.jpg", "start": 6360068, "end": 6412266}, {"filename": "/Skyboxes/skybox1/skybox-nz.jpg", "start": 6412266, "end": 6466840}, {"filename": "/Skyboxes/skybox1/skybox-px.jpg", "start": 6466840, "end": 6526890}, {"filename": "/Skyboxes/skybox1/skybox-py.jpg", "start": 6526890, "end": 6546524}, {"filename": "/Skyboxes/skybox1/skybox-pz.jpg", "start": 6546524, "end": 6605392}, {"filename": "/Skyboxes/skybox1/skybox.jpg", "start": 6605392, "end": 7356764}, {"filename": "/Skyboxes/skybox2/negx.jpg", "start": 7356764, "end": 7701944}, {"filename": "/Skyboxes/skybox2/negy.jpg", "start": 7701944, "end": 8078335}, {"filename": "/Skyboxes/skybox2/negz.jpg", "start": 8078335, "end": 8455983}, {"filename": "/Skyboxes/skybox2/posx.jpg", "start": 8455983, "end": 8782540}, {"filename": "/Skyboxes/skybox2/posy.jpg", "start": 8782540, "end": 8888507}, {"filename": "/Skyboxes/skybox2/posz.jpg", "start": 8888507, "end": 9229484}, {"filename": "/Skyboxes/skybox2/skybox.jpg", "start": 9229484, "end": 10894825}, {"filename": "/Skyboxes/spiaggia_di_mondello_1k.hdr", "start": 10894825, "end": 12428067}, {"filename": "/Skyboxes/spiaggia_di_mondello_4k.hdr", "start": 12428067, "end": 37350233}, {"filename": "/Skyboxes/wintersun.jpg", "start": 37350233, "end": 37533249}, {"filename": "/Textures/Anisotropic/CarbonFiberAniso_albedo.png", "start": 37533249, "end": 38590689}, {"filename": "/Textures/Anisotropic/CarbonFiberAniso_anisotropyAngle.png", "start": 38590689, "end": 47004350}, {"filename": "/Textures/Anisotropic/CarbonFiberAniso_normal.png", "start": 47004350, "end": 53315946}, {"filename": "/Textures/Anisotropic/CarbonFiberAniso_orm.png", "start": 53315946, "end": 53791854}, {"filename": "/Textures/Anisotropic/vtk_Anisotropy.png", "start": 53791854, "end": 54454402}, {"filename": "/Textures/Isotropic/vtk_Base_Color.png", "start": 54454402, "end": 55549435}, {"filename": "/Textures/Isotropic/vtk_Material.png", "start": 55549435, "end": 57127116}, {"filename": "/Textures/Isotropic/vtk_Normal.png", "start": 57127116, "end": 58246764}, {"filename": "/Textures/Isotropic/vtk_dark_bkg.png", "start": 58246764, "end": 58335176}], "remote_package_size": 58335176});

  })();
