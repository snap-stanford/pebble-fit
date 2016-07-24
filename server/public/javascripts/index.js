var graphsDiv = $('#graphs')

var opts = {
  full_width: true,
  height: 300,
  right: 40,
  top: 50,
  animate_on_load: true,
  target: '#graph',
  x_accessor: 'time',
  y_accessor: 'steps',
  y_scale_type: 'log',
  missing_is_hidden: true,
  brushing: true,
  brushing_history: true,
}

function load (url) {
  d3.json('/' + url + '?watch=' + window.watch_token, function (obj) {
    if (obj.length === 3) {
      var data = []
      for (var day = 0; day < obj.length; day++) {
        var day_activity = obj[day].activities
        for (var j = 0; j < day_activity.length; j++) {
          var result = new Date(day_activity[j].time)
          result.setDate(result.getDate() + day) // add days
          day_activity[j].time = result
        }
        data.push(day_activity)
      }
      reload_graphic(false, data)
    } else {
      var data = obj.activities
      for (var i = 0; i < data.length; i++) {
        data[i].time = new Date(data[i].time)
        var markers = obj.events
        for (var j = 0; j < markers.length; j++) {
          markers[j].time = new Date(markers[j].time)
          markers[j].label = markers[j].type
          if (markers[j].data) {
            markers[j].label += ' ' + markers[j].data
          }
        }
      }
      reload_graphic(true, data, markers)
    }
  })
}

function reload_graphic (is_single, data, markers) {
  var options = JSON.parse(JSON.stringify(opts))
  options.brushing_interval = d3.time.minute
  options.color = '#26c1e4'
  if (is_single) {
    options.markers = markers
    options.legend = ['today']
  } else {
    options.legend = ['today', 'yesterday', 'day_before_yesterday']
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

$(function () {
  $(".modify-time-period-controls > .btn").first().trigger('click');  
})
