
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
      var PACKAGE_NAME = 'packaged_data/ExportBunny.data';
      var REMOTE_PACKAGE_BASE = 'ExportBunny.data';
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
Module['FS_createPath']("/", "ExportBunny", true, true);

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
          }          Module['removeRunDependency']('datafile_packaged_data/ExportBunny.data');

      };
      Module['addRunDependency']('datafile_packaged_data/ExportBunny.data');

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
    loadPackage({"files": [{"filename": "/ExportBunny/ExportBunny_0.vtp", "start": 0, "end": 127363}, {"filename": "/ExportBunny/ExportBunny_1.vtp", "start": 127363, "end": 254687}, {"filename": "/ExportBunny/ExportBunny_10.vtp", "start": 254687, "end": 382027}, {"filename": "/ExportBunny/ExportBunny_11.vtp", "start": 382027, "end": 509472}, {"filename": "/ExportBunny/ExportBunny_12.vtp", "start": 509472, "end": 636903}, {"filename": "/ExportBunny/ExportBunny_13.vtp", "start": 636903, "end": 764239}, {"filename": "/ExportBunny/ExportBunny_14.vtp", "start": 764239, "end": 891581}, {"filename": "/ExportBunny/ExportBunny_15.vtp", "start": 891581, "end": 1019026}, {"filename": "/ExportBunny/ExportBunny_16.vtp", "start": 1019026, "end": 1146458}, {"filename": "/ExportBunny/ExportBunny_17.vtp", "start": 1146458, "end": 1273753}, {"filename": "/ExportBunny/ExportBunny_18.vtp", "start": 1273753, "end": 1401096}, {"filename": "/ExportBunny/ExportBunny_19.vtp", "start": 1401096, "end": 1528538}, {"filename": "/ExportBunny/ExportBunny_2.vtp", "start": 1528538, "end": 1655860}, {"filename": "/ExportBunny/ExportBunny_20.vtp", "start": 1655860, "end": 1783260}, {"filename": "/ExportBunny/ExportBunny_21.vtp", "start": 1783260, "end": 1910617}, {"filename": "/ExportBunny/ExportBunny_22.vtp", "start": 1910617, "end": 2037972}, {"filename": "/ExportBunny/ExportBunny_23.vtp", "start": 2037972, "end": 2165428}, {"filename": "/ExportBunny/ExportBunny_24.vtp", "start": 2165428, "end": 2292876}, {"filename": "/ExportBunny/ExportBunny_25.vtp", "start": 2292876, "end": 2420231}, {"filename": "/ExportBunny/ExportBunny_26.vtp", "start": 2420231, "end": 2547584}, {"filename": "/ExportBunny/ExportBunny_27.vtp", "start": 2547584, "end": 2675042}, {"filename": "/ExportBunny/ExportBunny_28.vtp", "start": 2675042, "end": 2802484}, {"filename": "/ExportBunny/ExportBunny_29.vtp", "start": 2802484, "end": 2929837}, {"filename": "/ExportBunny/ExportBunny_3.vtp", "start": 2929837, "end": 3057266}, {"filename": "/ExportBunny/ExportBunny_30.vtp", "start": 3057266, "end": 3184619}, {"filename": "/ExportBunny/ExportBunny_31.vtp", "start": 3184619, "end": 3312021}, {"filename": "/ExportBunny/ExportBunny_32.vtp", "start": 3312021, "end": 3439451}, {"filename": "/ExportBunny/ExportBunny_33.vtp", "start": 3439451, "end": 3566794}, {"filename": "/ExportBunny/ExportBunny_34.vtp", "start": 3566794, "end": 3694137}, {"filename": "/ExportBunny/ExportBunny_35.vtp", "start": 3694137, "end": 3821585}, {"filename": "/ExportBunny/ExportBunny_36.vtp", "start": 3821585, "end": 3949035}, {"filename": "/ExportBunny/ExportBunny_37.vtp", "start": 3949035, "end": 4076394}, {"filename": "/ExportBunny/ExportBunny_38.vtp", "start": 4076394, "end": 4203751}, {"filename": "/ExportBunny/ExportBunny_39.vtp", "start": 4203751, "end": 4331211}, {"filename": "/ExportBunny/ExportBunny_4.vtp", "start": 4331211, "end": 4458642}, {"filename": "/ExportBunny/ExportBunny_40.vtp", "start": 4458642, "end": 4586088}, {"filename": "/ExportBunny/ExportBunny_41.vtp", "start": 4586088, "end": 4713443}, {"filename": "/ExportBunny/ExportBunny_42.vtp", "start": 4713443, "end": 4840800}, {"filename": "/ExportBunny/ExportBunny_43.vtp", "start": 4840800, "end": 4968265}, {"filename": "/ExportBunny/ExportBunny_44.vtp", "start": 4968265, "end": 5095713}, {"filename": "/ExportBunny/ExportBunny_45.vtp", "start": 5095713, "end": 5223068}, {"filename": "/ExportBunny/ExportBunny_46.vtp", "start": 5223068, "end": 5350430}, {"filename": "/ExportBunny/ExportBunny_47.vtp", "start": 5350430, "end": 5477895}, {"filename": "/ExportBunny/ExportBunny_48.vtp", "start": 5477895, "end": 5605327}, {"filename": "/ExportBunny/ExportBunny_49.vtp", "start": 5605327, "end": 5732664}, {"filename": "/ExportBunny/ExportBunny_5.vtp", "start": 5732664, "end": 5859956}, {"filename": "/ExportBunny/ExportBunny_50.vtp", "start": 5859956, "end": 5987299}, {"filename": "/ExportBunny/ExportBunny_51.vtp", "start": 5987299, "end": 6114745}, {"filename": "/ExportBunny/ExportBunny_52.vtp", "start": 6114745, "end": 6242187}, {"filename": "/ExportBunny/ExportBunny_53.vtp", "start": 6242187, "end": 6369540}, {"filename": "/ExportBunny/ExportBunny_54.vtp", "start": 6369540, "end": 6496893}, {"filename": "/ExportBunny/ExportBunny_55.vtp", "start": 6496893, "end": 6624297}, {"filename": "/ExportBunny/ExportBunny_56.vtp", "start": 6624297, "end": 6751743}, {"filename": "/ExportBunny/ExportBunny_57.vtp", "start": 6751743, "end": 6879096}, {"filename": "/ExportBunny/ExportBunny_58.vtp", "start": 6879096, "end": 7006452}, {"filename": "/ExportBunny/ExportBunny_59.vtp", "start": 7006452, "end": 7133915}, {"filename": "/ExportBunny/ExportBunny_6.vtp", "start": 7133915, "end": 7261255}, {"filename": "/ExportBunny/ExportBunny_60.vtp", "start": 7261255, "end": 7388699}, {"filename": "/ExportBunny/ExportBunny_61.vtp", "start": 7388699, "end": 7515996}, {"filename": "/ExportBunny/ExportBunny_62.vtp", "start": 7515996, "end": 7643354}, {"filename": "/ExportBunny/ExportBunny_63.vtp", "start": 7643354, "end": 7770815}, {"filename": "/ExportBunny/ExportBunny_7.vtp", "start": 7770815, "end": 7898252}, {"filename": "/ExportBunny/ExportBunny_8.vtp", "start": 7898252, "end": 8025679}, {"filename": "/ExportBunny/ExportBunny_9.vtp", "start": 8025679, "end": 8153019}], "remote_package_size": 8153019});

  })();
