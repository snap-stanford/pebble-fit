var mongoose = require('mongoose');

var userSchema = new mongoose.Schema({
  watch             : { type: String, required: true },
  group             : { type: String, required: true },
  name              : { type: String },
  email             : { type: String },
  age               : { type: String },
  gender            : { type: String },
  heightCM          : { type: String },
  heightFT          : { type: String },
  heightIN          : { type: String },
  heightU           : { type: String },
  weight            : { type: String },
  weightU           : { type: String },
  race              : { type: String },
  school            : { type: String },
  occupation        : { type: String },
  deskwork          : { type: String },
  income            : { type: String },
  country           : { type: String },
  zipcode           : { type: String },
  sit1              : { type: String },
  sit2              : { type: String },
  sit3              : { type: String },
  sit4              : { type: String },
  sit5              : { type: String },
  sit6              : { type: String },
  sit7              : { type: String },
  sit8              : { type: String },
  sit8T             : { type: String },
  modifiedAt        : { type: Date, required: false, default: Date.now },
  createdAt         : { type: Date, required: true, default: Date.now },
  configUpdatedAt   : { type: Date, required: true, default: Date.now }
});

module.exports = mongoose.model('User', userSchema);
