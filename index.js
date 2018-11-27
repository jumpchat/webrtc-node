var binary = require('node-pre-gyp');
var path = require('path');
try {
    var binding_path = binary.find(path.resolve(path.join(__dirname,'./package.json')), { debug: false });
    var binding = require(binding_path);
    module.exports = binding;
} catch (e) {
    var binding_path = binary.find(path.resolve(path.join(__dirname,'./package.json')), { debug: true });
    var binding = require(binding_path);
    module.exports = binding;
}
