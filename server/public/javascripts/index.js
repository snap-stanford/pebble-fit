var graphsDiv = $('#graphs')

var createGraph = function (param, legend) {
  g = document.createElement('div')
  g.setAttribute('id', param)
  graphsDiv.append(g)

  d3.json('/' + param+ '?watch=6147d09748dd323ff6d0a3cb50b593db', function(data) {
      for (var i = 0; i < data.length; i++) {
        if($.isArray(data[i])) {
          console.log(data[i][0])
          console.log(new Date(data[i][0].time))
          for (var j = 0; j < data[i].length; j++) {
            var result = new Date(data[i][j].time)
            result.setDate(result.getDate() + i) // add days
            data[i][j].time = result
          }
        } else {
          data[i].time = new Date(data[i].time)
        }
      }
      data = MG.convert.number(data, 'steps')

      var options = {
          data: data,
          width: $(g).width(),
          height: 400,
          right: 40,
          target: '#' + param,
          x_accessor: 'time',
          y_accessor: 'steps',
      }

      if($.isArray(data[0])) {
        console.log('here')
        legendDiv = document.createElement('div') 
        legendDiv.setAttribute('id', param+'legend')
        $('#' + param).append(legendDiv)
        options['legend'] = legend
        options['legend_target'] = '#' + param + 'legend'
      }
      console.log(options)

      MG.data_graphic(options)
  })
}

var graphs = ['latest_hour', 'today', 'last_3_days']
for (var i = 0; i < graphs.length; i++) {
    createGraph(graphs[i], ['today', 'yesterday', 'day_before_yesterday'])
}
