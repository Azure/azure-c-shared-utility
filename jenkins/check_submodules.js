#!/usr/bin/env node
// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

// USAGE
// check_submodule.js [local path to test against] [branch to merge into]

// MOTIVATION AND BACKGROUND
// See https://www.npmjs.com/package/check_submodules 


var exec = require('child_process').exec;
var path = require('path');

// Determines the number of commits the current repoRoot is ahead of its master (or main) branch.
function getCommitsAheadOfMasterOrMainCount(repoRoot, callback) {
  // This is a two part command encapsulated in a single exec() to simplify Node callback handling.
  // * "git remote show ... cut -d ..." queries the given repo and determines its default branch - typically
  //    either "master" or "main".  This simple string is returned from this operation to the next step...
  // * "git rev-list --left-right --count origin/<master or main>...<repoRoot.Hash>" returns how
  //    repoRoot.Hash (the commit we are submoduled against) relates to the master|main.
  exec('git rev-list --left-right --count origin/$(git remote show ' + repoRoot.remoteUrl + ' | grep \'HEAD branch\' | cut -d\' \' -f5)...' + repoRoot.hash, { cwd: repoRoot.path }, function (err, stdout, stderr) {
    if (!err) {
      var j = stdout.split("\t");
      if (j.length >= 2) {
        var commitsAhead = parseInt(j[1], 10);
        callback(null, commitsAhead);
      } else {
        callback("bad data from git rev-list");
      };
    } else {
      callback(err);
    }
  });
}

// getRemote returns the URL for current path
function getRemote(path, callback) {
  exec('git remote get-url origin', { cwd: path }, function (err, stdout, stderr) {
    if (!err) {
      callback(null, stdout.trim());
    } else {
      callback(err);
    }
  });
}

// Recursively descend through submodules of currently visited repo
function getSubmodules(repoRoot, callback) {
  exec('git submodule status', { cwd: repoRoot.path }, function (err, stdout, stderr) {
    if (!err) {
      var results = [];
      if (!!stdout) {
        var submodules = stdout.trim().split('\n');
        submodules.forEach(function (sm) {
          var smDetails = sm.trim().split(' ');
          var smDesc = {
            name: smDetails[1],
            path: path.join(repoRoot.path, smDetails[1]),
            hash: smDetails[0]
          };
          results.push(smDesc);
        });
      }

      // Don't allow submodules that are ahead of their own master|main if we're checking in to master|main
      // repoRoot.hash is empty for the root of the tree, which is allowed to be ahead of its master|main.
      if (mergingToMasterOrMain && repoRoot.hash != '') {
        getCommitsAheadOfMasterOrMainCount(repoRoot, function (err, commitsAhead) {
          if (!err) {
            repoRoot.commitsAhead = commitsAhead;
            if (commitsAhead > 0) {
              returnCode = -1;
            }
            callback(null, results);
          } else {
            callback(err);
          };
        });
      } else {
        callback(null, results);
      }
    } else {
      callback(err);
    }
  });
}

// handleGetSubmoduleResults is invoked while getSubmodules retrieves submodule information.
// It performs initial checking and recursively descends submodules under this module, if any.
function handleGetSubmoduleResults(err, results, treeNode, hashTable, callback) {
if (err) {
  callback(err);
} else {
  treeNode.children = results;
  treeNode.childCounter = treeNode.children.length;
  if (treeNode.childCounter > 0) {
    treeNode.children.forEach(function (child) {
      if (child.hash[0] === '-' || child.hash[0] === '+') {
        // submodule is not initialized or has uncommitted changes
        console.error('Submodule tree is not initialized - please verify that you have recursively cloned and that there are not uncommitted changes in submodules. Consider running \'git submodule update --init --recursive\'');
        process.exit(1);
      } else {
        getSubmoduleTree(child, hashTable, function (err, results) {
          if (err) {
            callback(err);
          } else {
            treeNode.childCounter--;
            if (treeNode.childCounter === 0) {
              delete treeNode.childCounter;
              callback();
            }
          }
        });
      }
    });
  } else {
    delete treeNode.children;
    delete treeNode.childCounter;
    callback();
  }
}
}

// Collect a tree representing the repository submodules
function getSubmoduleTree(treeNode, hashTable, callback) {
  getRemote(treeNode.path, function (err, remoteUrl) {
    treeNode.remoteUrl = remoteUrl;
    if (!hashTable.hasOwnProperty(remoteUrl)) {
      // This is the first time we've encountered this URL.
      // Add it to the hash table.
      hashTable[remoteUrl] = treeNode;
    } else if (hashTable[remoteUrl].hash !== treeNode.hash) {
      // We have seen this URL before AND its commitHash is not
      // expected.  Mark this as an error, though continue
      // test run for any additional reports.
      treeNode.isNotSynced = true;
      hashTable[remoteUrl].isNotSynced = true;
      returnCode = -1;
    }

    getSubmodules(treeNode, function (err, results) {
      handleGetSubmoduleResults(err, results, treeNode, hashTable, callback);
    });
  });

  
}

var offsetChar = ' ';
var offsetUnit = 2;
function printSubmoduleTree(treeNode, offset) {
  var offsetString = '';
  for (var i = 0; i < offset; i++) {
    offsetString += offsetChar;
  }

  var line = offsetString + treeNode.name + ': ' + treeNode.hash;
  console.log(line);
  if (treeNode.isNotSynced || treeNode.commitsAhead > 0) {
    var errorLine = ' *** FAILURE *** In line above ';
    if (treeNode.isNotSynced) {
      errorLine += ' Error: submodule mismatch ';
    };
    if (treeNode.commitsAhead > 0) {
      errorLine += ' Error: submodule is ' + treeNode.commitsAhead + " commits ahead of master";
    }
    console.log(errorLine);
  }
  if (treeNode.children) {
    var childOffset = offset + offsetUnit;
    treeNode.children.forEach(function (childNode) {
      printSubmoduleTree(childNode, childOffset);
    })
  }
}

var returnCode = 0;
var nodeCounter = 0;
var tree = {
  name: path.parse(process.argv[2]).name,
  path: process.argv[2],
  hash: '',
  children: []
};

var hashTable = {};

if (process.argv.length < 4) {
  console.log("Usage is check_submodule.js <git-repo-path> <branch-to-merge-to>");
  console.error("!!FAILED!! check_submodule.js requires two parameters");
  process.exit(1);
}

var mergingToMasterOrMain = (process.argv[3].toLowerCase() === "master") || (process.argv[3].toLowerCase() === "main");

// "main" function - getSubmoduleTree recursively descends through potentially nested
// submodules, doing error checking along the way.
getSubmoduleTree(tree, hashTable, function (err, result) {
  if (err) {
    console.error(err);
  } else {
    printSubmoduleTree(tree, 0);
    process.exit(returnCode);
  }
});

