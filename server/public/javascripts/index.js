var graphsDiv = $('#graphs')

var opts = {
  full_width: true,
  height: 300,
  right: 40,
  left: 80,
  top: 50,
  animate_on_load: true,
  target: '#graph',
  x_accessor: 'time',
  y_accessor: 'steps',
  missing_is_hidden: true,
  y_extended_ticks: true,
  y_scale_type: 'log',
  y_label: 'log steps',
  brushing: true,
  brushing_history: true
}

globals = {}

function load (url) {
  d3.json('/' + url + '?watch=' + window.watch_token, function (response) {
    var obj = response.data
    var data = markers = null
    var multiple = response.type === 'multiple'
    if (multiple) {
      data = []
      for (var day = 0; day < obj.length; day++) {
        var day_activity = obj[day].activities
        for (var j = 0; j < day_activity.length; j++) {
          var result = new Date(day_activity[j].time)
          result.setDate(result.getDate() + day) // add days
          day_activity[j].time = result
        }
        data.push(day_activity)
      }
    } else {
      var data = obj.activities
      for (var i = 0; i < data.length; i++) {
        data[i].time = new Date(data[i].time)
      }
    
      // FIXME: this is a workaround for displaying a whole day range of onto browser,
      // by adding a artificial data point at the current time and another at the 
      // start of the day. Note that this might affect the real data if there is 
      // actually some data collected at these 2 timestamp points.
      var sod = new Date();
      sod.setHours(6,0,0,0);
      if (url === 'latest_day') {
          data.push({time: sod, steps: 0});
      }
      data.push({time: new Date(), steps: 0});

      var markers = obj.events
      for (var j = 0; j < markers.length; j++) {
        markers[j].time = new Date(markers[j].time)
        markers[j].label = markers[j].type
        if (markers[j].data) {
          markers[j].label += ' ' + markers[j].data
        }
      }
    }

    globals.multiple = multiple
    globals.data = data
    globals.markers = markers
    reload_graphic(multiple, data, markers)

  })
}

function reload_graphic (multiple, data, markers) {
  var checkbox = $('.checkbox.showMarkers input')
  checkbox.attr("disabled", multiple)
  checkbox.prop("checked", !multiple)

  var options = JSON.parse(JSON.stringify(opts))
  options.brushing_interval = d3.time.minute
  options.colors = ['#26c1e4']
  if (multiple) {
    options.legend = ['today']
    for (var i = 1; i < data.length; i++) {
      options.legend.push(i + ' days ago')
    }
  } else {
    options.legend = ['today']
    if(checkbox.is(':checked')) {
        options.markers = markers
    }
  }
  options.legend_target = '#legend'
  options.data = MG.convert.number(data, 'steps')
  MG.data_graphic(options)
}

$('.modify-time-period-controls button').on('click', function () {
  var time_period = $(this).data('time_period')
  load(time_period)
  $(this).addClass('active').siblings().removeClass('active')
})

$('.modify-y-scale-controls button').on('click', function () {
  $(this).addClass('active').siblings().removeClass('active')
  var scale = $(this).data('scale')
  opts.y_scale_type = scale
  opts.y_label = scale + ' steps'
  reload_graphic(globals.multiple, globals.data, globals.markers)
})

$('.checkbox.showMarkers input').change(function () {
  reload_graphic(
    globals.multiple,
    globals.data,
    $(this).is(':checked') ? globals.markers : null)
})

$(function () {
  $('.modify-time-period-controls > .btn').first().trigger('click')
  // $('.modify-y-scale-controls > .btn').first().trigger('click')
})
