const test = require('node:test');
const assert = require('node:assert/strict');
const vm = require('node:vm');
const fs = require('node:fs');
const path = require('node:path');

function loadApp() {
    let tick;
    const calls = [];
    const context = {
        $: {ajax() {}}, VueI18n: function () {},
        get_local_lang: () => 'en', get_local_val: (p, k, d) => d,
        range: () => [], get_id: () => 1,
        window: {addEventListener() {}}, clearInterval() {},
        setInterval(fn, ms) { tick = fn; calls.push(ms); return calls.length; },
    };
    vm.createContext(context);
    vm.runInContext(fs.readFileSync(path.join(__dirname, '../www/js/app.js'), 'utf8'), context);
    const App = vm.runInContext('App', context);
    const app = Object.assign(App.data(), App.methods, {$t: x => x});
    return {app, tick: () => tick(), calls, context};
}

test('changing refresh frequency keeps both data streams updating', () => {
    const {app, tick, calls} = loadApp();
    let battery = 0, config = 0;
    app.ipc_send_wrapper = () => {};
    app.get_bat_info = () => battery++;
    app.get_conf = () => config++;
    app.change_update_freq(20);
    tick();
    assert.equal(battery, 1);
    assert.equal(config, 1);
    app.change_update_freq(60);
    tick();
    assert.deepEqual(calls, [20000, 60000]);
    assert.equal(battery, 2);
});

test('config refresh adopts another client frequency without duplicate timers', () => {
    const {app, calls} = loadApp();
    app.get_bat_info = () => {};
    app.old_temp_mode = 0;
    const reply = {data: {update_freq: 20}};
    app.get_conf_cb(reply);
    app.get_conf_cb(reply);
    app.get_conf_cb({data: {update_freq: 60}});
    assert.deepEqual(calls, [20000, 60000]);
});

test('recovery is sent once while pending and failed actions remain visible', () => {
    const {app} = loadApp();
    const requests = [];
    app.ipc_send_wrapper = req => requests.push(req);
    app.get_conf = app.get_bat_info = () => {};
    app.restore_charging();
    app.restore_charging();
    assert.equal(requests.length, 1);
    assert.equal(requests[0].api, 'restore_charging');
    app.control_action_cb({status: -2});
    assert.equal(app.control_busy, false);
    assert.equal(app.msg_list[0].type, 'error');
    app.restore_charging();
    assert.equal(requests.length, 2);
});

test('manual controls refresh automatic enable state after the response', () => {
    const {app} = loadApp();
    let request;
    app.ipc_send_wrapper = req => request = req;
    app.set_inflow_status(true);
    assert.equal(request.callback, 'window.app.control_action_cb');
    app.set_charge_status(false);
    assert.equal(request.callback, 'window.app.set_charge_status_cb');
});

test('battery current wins over writable charging flag; absent adapter is supported', () => {
    const {app} = loadApp();
    app.get_bat_info_cb({status: 0, data: {IsCharging: true, InstantAmperage: 0}});
    assert.equal(app.is_charge(), false);
    assert.equal(app.is_inflow(), false);
    assert.equal(app.get_adaptor_desc(), '—');
    app.bat_info.InstantAmperage = 500;
    assert.equal(app.is_charge(), true);
    assert.equal(app.get_health({}), '—');
});

test('unconfirmed stop and offline service show errors', () => {
    const {app} = loadApp();
    app.daemon_alive = true;
    app.control = {state: 'stop_unconfirmed'};
    assert.equal(app.control_alert_type(), 'error');
    assert.equal(app.control_title(), 'control_state_stop_unconfirmed');
    app.daemon_alive = false;
    assert.equal(app.control_title(), 'control_service_unavailable');
});
