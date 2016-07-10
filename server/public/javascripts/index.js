var graphsDiv = $('#graphs')

var createGraph = function (param, legend) {
  g = document.createElement('div')
  g.setAttribute('id', param)
  g.setAttribute('class', 'graph')
  graphsDiv.append(g)
  d3.json('/' + param+ '?watch=' + window.watch_token, function(data) {
      for (var i = 0; i < data.length; i++) {
        if($.isArray(data[i])) {
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
          title: param.replace(/_/g, ' '),
          data: data,
          width: $(g).width(),
          height: 400,
          right: 40,
          target: '#' + param,
          x_accessor: 'time',
          y_accessor: 'steps',
          interpolate: 'basic',
      }

      if($.isArray(data[0])) {
        legendDiv = document.createElement('div') 
        legendDiv.setAttribute('id', param+'legend')
        $('#' + param).append(legendDiv)
        options['legend'] = legend
        options['legend_target'] = '#' + param + 'legend'
      } else {
        options['color'] = '#43F436'
      }

      MG.data_graphic(options)
  })
}

var graphs = ['latest_hour', 'today', 'last_3_days']
for (var i = 0; i < graphs.length; i++) {
    createGraph(graphs[i], ['today', 'yesterday', 'day_before_yesterday'])
}
